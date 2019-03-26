/*	Copyright  (c)	Günter Woigk 2008 - 2019
                    mailto:kio@little-bat.de

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    Permission to use, copy, modify, distribute, and sell this software and
    its documentation for any purpose is hereby granted without fee, provided
    that the above copyright notice appear in all copies and that both that
    copyright notice and this permission notice appear in supporting
    documentation, and that the name of the copyright holder not be used
    in advertising or publicity pertaining to distribution of the software
    without specific, written prior permission.  The copyright holder makes no
    representations about the suitability of this software for any purpose.
    It is provided "as is" without express or implied warranty.

    THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
    INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
    EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
    CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
    DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
    PERFORMANCE OF THIS SOFTWARE.
*/


#define SAFE	3
#define LOG		0


#include "config.h"
#include <pthread.h>
#include "tempmem.h"


static pthread_key_t tempmem_key;			// key for per-thread TempMemPool
#define ALIGNMENT_MASK	(_MAX_ALIGNMENT-1)
#define MAX_REQ_SIZE	500
#define BUFFER_SIZE		4000


/* ----	Deallocate pool ----------------------
        called at thread termination
        while pointer in tempmem_key is != nullptr.
        This should happen only once, for
        the automatically created pool.
*/
static void deallocate_pool(void* pool)
{
    delete static_cast<TempMemPool*>(pool);
}


/* ----	Initialization ---------------------------------
*/
static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static volatile bool  virgin	   = 1;				// for faster init test

static void create_key(void)
{
    int err = pthread_key_create( &tempmem_key, deallocate_pool );
    if(err) abort( "init TempMemPool: %s", strerror(err) );
}

static void init()
{
    XLogIn("init TempMemPool");

// initialize: get a pthread_key:
    int err = pthread_once( &once_control, create_key );
    XXXASSERT(err==0);

    virgin = false;
}


/* ---- TempMemPool member functions --------------------
*/
TempMemPool::TempMemPool()
:	size(0),
    data(nullptr)
{
    XXLogIn("new TempMemPool");

    if(virgin) init();

    prev = static_cast<TempMemPool*> (pthread_getspecific( tempmem_key ));
    XXXLogLine("  prev pool = %lx", ulong(prev));
    XXXLogLine("  this pool = %lx", ulong(this));
    //int err =
    pthread_setspecific( tempmem_key, this );				// may fail with ENOMEM (utmost unlikely)
    //if(err) { Abort("new TempMemPool: ",strerror(err)); }	// in which case this pool is not registered
}															// and GetPool() keeps using the outer pool


TempMemPool::~TempMemPool()
{
    XXLogIn("delete TempMemPool");
    XXXLogLine("  this pool = %lx", ulong(this));
    XXXLogLine("  prev pool = %lx", ulong(prev));

    purge();
    int err = pthread_setspecific( tempmem_key, prev );		// may fail with ENOMEM (utmost unlikely)
    if(err) { abort("delete TempMemPool: %s",strerror(err)); }
}


void TempMemPool::purge()				// Purge() == destroy + create pool
{
    XXLogIn("TempMemPool::Purge");
    XXXLog("  this pool = %lx ", ulong(this));

    while( data!=nullptr )
    {
        XXXLog(".");
        TempMemData* prev = data->prev;
        delete[] reinterpret_cast<char*>(data); data = prev;
    }
    size = 0;
    XXXLogLine(" ok");
}


char* TempMemPool::alloc( int bytes ) noexcept(false)
{
    if( bytes<=size )					// fits in current buffer?
    {
        size -= bytes;
        return data->data + size;
    }
    else if( bytes<=MAX_REQ_SIZE )		// small request?
    {
        TempMemData* newdata = reinterpret_cast<TempMemData*> ( new char[ sizeof(TempMemData) + 4000 ] );
        XXXLogLine("tempmem new data = $%lx", ulong(newdata));
        XXXASSERT( (uintptr_t(newdata) & ALIGNMENT_MASK) == 0 );
        newdata->prev = data;
        data = newdata;
        size = 4000-bytes;
        return newdata->data + size;
    }
    else								// large request
    {
        TempMemData* newdata = reinterpret_cast<TempMemData*> ( new char[ sizeof(TempMemData) + uint(bytes) ] );
        XXXLogLine("tempmem new data = $%lx", ulong(newdata));
        XXXASSERT( (uintptr_t(newdata) & ALIGNMENT_MASK) == 0 );
        if(data)
        {
            newdata->prev = data->prev;		// neuen Block 'unterheben'
            data->prev = newdata;
        }
        else
        {
            newdata->prev = nullptr;
            data = newdata;
            size = 0;
        }
        return newdata->data;
    }
}


char* TempMemPool::allocMem( int bytes ) noexcept(false)
{
    char* p = alloc(bytes);
    if( data->prev && p == data->prev->data )	// wurde "large request" 'untergehoben' ?
    {
        return p;
    }
    else
    {
        int n = size&ALIGNMENT_MASK;
        size -= n;
        return p-n;
    }
}


/* ---- Get the current temp mem pool -------------------------
        if there is no pool, then it is created.
*/
TempMemPool* TempMemPool::getPool() noexcept(false)
{
    if(virgin) init();
    TempMemPool* pool = static_cast<TempMemPool*> ( pthread_getspecific(tempmem_key) );
    return pool ? pool : new TempMemPool();
}


/* ---- Get the surrounding temp mem pool -------------------------
        if there is no surrounding pool, then it is created.
        a 'current pool' should be in place, else 2 pools are created.
*/
TempMemPool* TempMemPool::getXPool() noexcept(false)
{
    TempMemPool* pool = getPool();
    TempMemPool* prev = pool->prev;
    if( !prev )
    {
        prev = new TempMemPool();					// automatically create 'outer' pool
        prev->prev = nullptr;							// 'outer' pool 'unterheben'.
        pool->prev = prev;
        pthread_setspecific( tempmem_key, pool );	// aktuellen Pool erneut als 'aktuell' markieren
    }												// note: *might* fail with ENOMEM (utmost unlikely)
    return prev;									// in which case we keep on using the outer pool
}


/* ---- Get a temp cstring -------------------------
*/
char* tempstr( int len ) noexcept(false)
{
    return TempMemPool::getPool()->allocStr(len);
}


/* ---- Get a temp cstring -------------------------
        from the surrounding pool
*/
char* xtempstr( int len ) noexcept(false)
{
    return TempMemPool::getXPool()->allocStr(len);
}


/* ---- Get memory for temp. usage -------------------------
*/
char* tempmem( int size ) noexcept(false)
{
    return TempMemPool::getPool()->allocMem(size);
}


/* ---- Get memory for temp. usage -------------------------
        from the surrounding pool
*/
char* xTempMem( int size ) noexcept(false)
{
    return TempMemPool::getXPool()->allocMem(size);
}


/* ---- Purge current pool -------------------------
*/
void purgeTempMem()
{
    TempMemPool::getPool()->purge();
}




#if XXSAFE
namespace TempMemTest
{
    static struct T
    {
        T()
        {
            // check assumptions:
            ASSERT( (sizeof(TempMemData)&(ALIGNMENT_MASK)) == 0 );
            ptr p1 = new char[17], p2 = new char[15];
            ASSERT( (uintptr_t(p1)&ALIGNMENT_MASK)==0 );
            ASSERT( (uintptr_t(p2)&ALIGNMENT_MASK)==0 );
            delete[] p1;
            delete[] p2;
        }
    } dummy;
}
#endif









