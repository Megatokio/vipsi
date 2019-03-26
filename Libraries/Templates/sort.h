/*	Copyright  (c)	Günter Woigk 1998 - 2019
  					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



	Threshold Sorter (alternating buffer variant)		adaptable via #defines
	---------------------------------------------		----------------------

<German>
	Der zu sortierende Bereich wird in drei Felder unterteilt:
		1. vorsortierte Elemente unterhalb (<=) einer unteren Schranke
		2. noch unsortierte Elemente
		3. vorsortierte Elemente oberhalb (>=) einer oberen Schranke

	Start:
		Falls 1. Element > letztes Element  =>  diese beiden vertauschen
		Feld 1 := 1. Element
		Feld 3 := letztes Element
		Untere Schranke := 1. Element
		Obere Schranke  := letztes Element

	Danach wird abwechselnd das erste und das letzte Element des mittleren, noch
	unsortierten Feldes betrachtet:
		Element[i] <= untere Grenze => unterem Bereich zuordnen
		Element[i] >= obere Grenze  => oberem Bereich zuordnen
		Element[i] dazwischen =>
			dem unteren bzw. oberen Bereich zuordnen und Grenzwert entsprechend neu setzen

		Dabei wird dem Bereich der Vorrang gegeben, an den Element[i] grenzt:
		Wird also gerade das erste Element des mittleren Bereiches betrachtet,
		wird es vorzugsweise dem unteren sortierten Feld zugeschlagen, sonst anders rum.

		Damit muss das Element gar nicht verschoben/vertauscht werden und durch das
		alternieren wird die Wahrscheinlichkeit für eine symmetrische Größe der vorsortierten
		Bereiche erhöht. (sie ist aber leider nicht besonders hoch.)

	Ist der gesamte Bereich vorsortiert, also das mittlere Feld leer, werden die beiden
	vorsortierten Felder rekursiv nach gleichem Schema sortiert.


	Vorteile:
		nicht quadratischer Zeitzuwachs
		benötigt keinen temp array
		nur das Vergleichs-Makro CMP(A,B) muss implementierbar sein
		(man kann damit also alles sortieren, was überhaupt sortierbar ist.)

	Nachteil:
		Die Reihgenfolge von Elementen mit gleichem Wert wird verwürfelt
		(also ungeeignet für's Sortieren von Datenbanken nach 2 oder mehr Spalten)
</German>



	U S A G E
	---------

	this header file defines a sort() function
	based on the following macros:

		#define	SORTER	  		sort			// required: name of sort function
		#define TYPE	  		int				// required: data type in array 			1)
		#define CMP(A,B)		((A)>(B))		// required: compare function or macro		2,3)
		#define SWAP(A,B)		swap(A,B)		// optional: swap two items
		#define FUNCTION_SORTER					// optional: sort(T*,T*,compare_function*)	3,4)

		1) TYPE may be undefined for template definitions which uses TYPE as class name
		2) CMP must be undefined if FUNCTION_SORTER is defined
		3) The compare function must return true if the two arguments must be swapped.
		   For sorting in ascending order the means 'greater than':  bool CMP(A,B) {return A>B;}
		4) The signature of the compare function is:  bool(*)(TYPE&,TYPE&)

	sorter for floats:

			#define 	SORTER		sort
			#define		TYPE		float
			#define 	CMP(A,B)	((A)>(B))
			#include	"sort.h"

	sorter for cstrings:

			#define 	SORTER		sort_str
			#define		TYPE		char*
			#define		CMP(A,B)	(strcmp(A,B)>0)
			#include	"sort.h"

	member function of class Var to sort array of pointers to Var:

			#define 	SORTER		Var::sort_array
			#define 	TYPE		Var*
			#define 	CMP(A,B)	(*(A)>*(B))
			#include	"sort.h"

	sorter template with special swap function:

			#define 	SORTER		xsort
			#define 	CMP(A,B)	(*(A)>*(B))
			#define		SWAP(A,B)	xswap(A,B)
			template<class TYPE>
			#include	"sort.h"

	sorter template with compare function:

			#define 	FUNCTION_SORTER
			#define 	SORTER 		sort
			template<class TYPE>
			#include 	"sort.h"

	sorter template for flat objects: (e.g. numbers)

			#define 	SORTER 		sort
			#define 	CMP(A,B)	((A)>(B))
			template<class TYPE>
			#include 	"sort.h"

	sorter template for pointers to objects:

			#define 	SORTER 		sort
			#define 	CMP(A,B)	(*(A)>*(B))
			template<class TYPE>
			#include 	"sort.h"
*/


#ifndef SORTER
#error SORTER not defined
#endif

#ifndef SWAP
#define SWAP(A,B)	kio::swap(A,B)
#endif



// ------------------------------------------------------------
//					Sort range [a ... [e
// ------------------------------------------------------------


#ifdef FUNCTION_SORTER
#undef FUNCTION_SORTER
#undef CMP
void SORTER( TYPE* a, TYPE* e, bool(*CMP)(TYPE&,TYPE&) ) noexcept(false)//throw(internal_error)
#else
void SORTER( TYPE* a, TYPE* e ) noexcept(false)//throw(internal_error)
#endif
{
	XASSERT(a&&e);									// both pointers not nullptr
	if(a>=e) return;
	XXXASSERT(a+(e-a)==e);							// alignment test: throws internal_error
	#define SORT(A,B)	if(CMP(A,B)){SWAP(A,B);}	// for convenience

	// ToDo-Stack:
	TYPE* a_stack[sizeof(void*)<<3];
	TYPE* e_stack[sizeof(void*)<<3];
	int pushed = 0;

	e--;	// use [a .. e] instead of [a .. [e

	for(;;)
	{
		XXXASSERT(a<=e);

		switch(e-a)
		{
		case 2: // 3 items
			SORT(a[0],a[2]);
			SORT(a[1],a[2]);
		case 1: // 2 items
			SORT(a[0],a[1]);
		case 0: // 1 item
			if(pushed)
			{
				pushed--;
				a = a_stack[pushed];
				e = e_stack[pushed];
				continue;
			}
			return;
		}

	// 4 or more elements

		TYPE* a0 = a;
		TYPE* e0 = e;

		SORT(*a,*e);

		TYPE* a_lim = a++;
		TYPE* e_lim = e--;

		for(;;)
		{
			if(a>=e) break;

			if(CMP(*a,*a_lim))
				if(CMP(*e_lim,*a))
					a_lim = a++;
				else
					SWAP(*a,*e), e--;
			else
				a++;

			if(a>=e) break;

			if(CMP(*e_lim,*e))
				if (CMP(*e,*a_lim))
					e_lim = e--;
				else
					SWAP(*a,*e), a++;
			else
				e--;
		}

	// letztes Element unten oder oben zuschlagen.				kio 2001-11-03
	// wäre unnötig, wenn in der schleife oben das abbruchkriterium (a>b) verwendet würde.
	// dann würde das letzte Element aber häufig mit sich selbst geswappt, was stören kann.
		if(CMP(*a,*a_lim)) e--; else a++;

	// größeren der subbereiche [a0..e] und [a..e0] pushen, den anderen sofort sortieren
	// das ist nötig, um stacküberlauf sicher zu vermeiden
		XXXASSERT(pushed < (int)NELEM(a_stack));
		if(e-a0 > e0-a)
		{
			a_stack[pushed] = a0;
			e_stack[pushed] = e;
			pushed++;

			a = e;
			e = e0;
		}
		else
		{
			a_stack[pushed] = a;
			e_stack[pushed] = e0;
			pushed++;

			a = a0;
	//		e = e;
		}
	}
}


#undef SORTER
#undef TYPE
#undef CMP
#undef SORT
#undef SWAP








