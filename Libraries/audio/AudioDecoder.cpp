/*	Copyright  (c)	Günter Woigk 2015 - 2015
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

	initially based on
		libaudiodecoder - Native Portable Audio Decoder Library
		libaudiodecoder API Header File
		http://www.oscillicious.com/libaudiodecoder
		Copyright (c) 2010-2012 Albert Santoni, Bill Good, RJ Ryan

	note:
		There is a bug in ExtFileAudio in OSX 10.4 (and maybe later)
		see: http://cocoadev.com/ExtAudioFile
		This results in a little offset of the sample seek position in ExtAudioFileSeek() for compressed files
		because the frames added for better decompression are not skipped.
		This offset is tiny, hopefully fixed today and will only add some silent samples at the beginning of the file.
		This is no longer fixed. I just call seek(0) in open() to avoid a mismatch after a later seek().
*/


#define SAFE 3
#define LOG 2
#include "AudioDecoder.h"



AudioDecoder::AudioDecoder()
:
	_filename(NULL),
	_num_frames(0),
	_frames_per_second(0),
	_num_channels(0),
	_sizeof_sample(0),
	_audiofile(NULL),
	_frame_position(0)
{
	bzero(&_clientformat,sizeof(_clientformat));
}


AudioDecoder::~AudioDecoder()
{
	close();
}


/*	close audio file.
	the decoder may be reused.
*/
OSStatus AudioDecoder::close()
{
	delete[] _filename;
	_filename = NULL;

	OSStatus e = _audiofile ? ExtAudioFileDispose(_audiofile) : ok;
	_audiofile = NULL;

	return e;
}


/*	open audio file for reading
	extracts various metrics
	e.g. you can then call numChannels(), numSamples(), samplesPerSecond(), sizeofSample()
*/
void AudioDecoder::open(cstr filename) throw(any_error)
{
	XLogIn("AudioDecoder::open(%s)",filename);

	// close existing file if open
    OSStatus err = close();

    // open the audio file
    CFStringRef urlStr = CFStringCreateWithCString(kCFAllocatorDefault, filename, kCFStringEncodingUTF8);
    CFURLRef urlRef = CFURLCreateWithFileSystemPath(NULL, urlStr, kCFURLPOSIXPathStyle, false);
    err = ExtAudioFileOpenURL(urlRef, &_audiofile);
    CFRelease(urlStr);
    CFRelease(urlRef);
	if(err) { _audiofile=NULL; throw any_error("AudioDecoder: Error opening file."); }
	_filename = newcopy(filename);
	_frame_position = 0;
	_clientformat.mBitsPerChannel = 0;

	// get the input file format
	uint32 size = sizeof(_fileformat);
	err = ExtAudioFileGetProperty(_audiofile, kExtAudioFileProperty_FileDataFormat, &size, &_fileformat);
	if(err) throw any_error("AudioDecoder: Error getting file format.");

	// get the total number of frames of the audio file
	// a frame is the set of samples with 1 sample per channel
	size = sizeof(_num_frames);
	err	= ExtAudioFileGetProperty(_audiofile, kExtAudioFileProperty_FileLengthFrames, &size, &_num_frames);
	if(err) throw any_error("AudioDecoder: Error getting number of frames.");

	_num_channels		= _fileformat.mChannelsPerFrame;
	_frames_per_second	= _fileformat.mSampleRate;
	_sizeof_sample		= _fileformat.mBytesPerFrame / _num_channels;
}


/*	set file position of audio file to desired sample position
*/
void AudioDecoder::seekSamplePosition(uint32 frame_position) throw(any_error)
{
    OSStatus err = ExtAudioFileSeek(_audiofile, frame_position);
	if(err) throw any_error(usingstr("AudioDecoder: seek(%u) failed: %i",(uint)frame_position, (int)err));
    _frame_position = frame_position;
}


/*	read buffer from file
	updates client data format if it has changed or is not yet set
	reads num_frames of num_channels of sizeof_sample into buffer bu[]
	note: if the file has more channels than requested, then the additional channels are simply ignored
		  if the file has less channels than requested, then the missing channels are cleared with silence
	returns number of frames actually read
*/
//private
uint32 AudioDecoder::read_buffer(void* bu, uint num_channels, uint32 num_frames, uint sizeof_sample) throw(any_error)
{
	if(_clientformat.mBitsPerChannel!=sizeof_sample<<3 || _clientformat.mChannelsPerFrame!=num_channels)
	{
		// setup output format:
		bzero(&_clientformat, sizeof(AudioStreamBasicDescription));
		_clientformat.mSampleRate		= _fileformat.mSampleRate;
		_clientformat.mFormatID			= kAudioFormatLinearPCM;
		_clientformat.mFormatFlags		= sizeof_sample==sizeof(float32) ?
					kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsFloat :
					kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
		_clientformat.mFramesPerPacket	= 1;
		_clientformat.mChannelsPerFrame = num_channels;
		_clientformat.mBitsPerChannel	= sizeof_sample * 8;
		_clientformat.mBytesPerFrame	= sizeof_sample * num_channels;
		_clientformat.mBytesPerPacket	= sizeof_sample * num_channels;

		// set output format on audio file:
		uint32 size  = sizeof(_clientformat);
		OSStatus err = ExtAudioFileSetProperty(_audiofile, kExtAudioFileProperty_ClientDataFormat, size, &_clientformat);
		if(err) throw any_error(usingstr("AudioDecoder: Error when setting ClientDataFormat: %i",(int)err));

		seekSamplePosition(_frame_position);	// security
	}

	AudioBufferList	audiobufferlist;
	audiobufferlist.mNumberBuffers = 1;
	audiobufferlist.mBuffers[0].mNumberChannels = num_channels;
	audiobufferlist.mBuffers[0].mDataByteSize = num_frames * num_channels * sizeof_sample;
	audiobufferlist.mBuffers[0].mData = bu;
	OSErr err = ExtAudioFileRead (_audiofile, &num_frames, &audiobufferlist);	// reads & sets num_frames
	if(err!=ok) throw any_error("AudioDecoder: failed to read from file");
	return num_frames;
}



/*	read samples from file into buffer
		num_channels may be 1 or 2 (mono or stereo)
		max_frames = buffer size = num_samples * num_channels
	if num_channels does not match, then channels are silently split or joined.
	If sample size is not yet set (_outputformat.mBitsPerChannel==0) or a previous read()
	was for a different sample size, then the output format is silently updated.
	returns number of frames actually read, which may be less than requested at file's end.
*/
uint32 AudioDecoder::read(int16* bu, uint32 max_frames, uint num_channels) throw(any_error)
{
	XXLogIn("AudioDecoder::read(int16,%u,%u)",(uint)max_frames,num_channels);
	XXASSERT(num_channels>=1 && num_channels<=7);

	typedef int16 sample;
	uint32 count,n;

	if(num_channels==1 && _num_channels>1)	// stereo|surround -> mono
	{
		sample* z = bu;						// destination pointer for channel conversion
		sample  qbu[1024*2];				// 1024 frames * 2 channels

		for(count=0; count<max_frames; count+=n)		// frames read
		{
			n = read_buffer(qbu, 2, min(max_frames-count,1024u), sizeof(sample));
			if(n==0) break;								// end of file

			for(sample *q=qbu,*e=q+n*2; q<e; q+=2)
			{
				*z++ = ((int32)*q + (int32)*(q+1)) / 2;
			}
		}
	}
	else					// requested and file's num channels match; or
	  						// stereo|surround to stereo|surround: default behaviour is ok; or
	{						// mono to stereo|surround: manually copy left to right channel
		for(count=0; count<max_frames; count+=n)		// frames read
		{
			n = read_buffer(bu+count*num_channels, num_channels, max_frames-count, sizeof(sample));
			if(n==0) break;								// end of file

			if(_num_channels==1 && num_channels>1)		// mono -> stereo|surround: only channel[0] was set
			{											// => copy left channel[0] to right channel[1]
				for(sample *q=bu+count*num_channels, *e=q+n*num_channels; q<e; q+=num_channels)
				{
					*(q+1) = *q; q += num_channels;
				}
			}
		}
	}

	_frame_position += count;
	return count;
}


/*	read samples from file into buffer
		num_channels may be 1 or 2 (mono or stereo)
		max_frames = buffer size = num_samples * num_channels
	if num_channels does not match, then channels are silently split or joined.
	If sample size is not yet set (_outputformat.mBitsPerChannel==0) or a previous read()
	was for a different sample size, then the output format is silently updated.
	returns number of frames actually read, which may be less than requested at file's end.
*/
uint32 AudioDecoder::read(float32* bu, uint32 max_frames, uint num_channels) throw(any_error)
{
	XXLogIn("AudioDecoder::read(float32,%u,%u)",(uint)max_frames,num_channels);
	XXASSERT(num_channels>=1 && num_channels<=7);

	typedef float32 sample;
	uint32 count,n;

	if(num_channels==1 && _num_channels>1)	// stereo|surround -> mono
	{
		sample* z = bu;						// destination pointer for channel conversion
		sample  qbu[1024*2];				// 1024 frames * 2 channels

		for(count=0; count<max_frames; count+=n)		// frames read
		{
			n = read_buffer(qbu, 2, min(max_frames-count,1024u), sizeof(sample));
			if(n==0) break;								// end of file

			for(sample *q=qbu,*e=q+n*2; q<e; q+=2)
			{
				*z++ = (*q + *(q+1)) / 2;
			}
		}
	}
	else					// requested and file's num channels match; or
	  						// stereo|surround to stereo|surround: default behaviour is ok; or
	{						// mono to stereo|surround: manually copy left to right channel
		for(count=0; count<max_frames; count+=n)		// frames read
		{
			n = read_buffer(bu+count*num_channels, num_channels, max_frames-count, sizeof(sample));
			if(n==0) break;								// end of file

			if(_num_channels==1 && num_channels>1)		// mono -> stereo|surround: only channel[0] was set
			{											// => copy left channel[0] to right channel[1]
				for(sample *q=bu+count*num_channels, *e=q+n*num_channels; q<e; q+=num_channels)
				{
					*(q+1) = *q; q += num_channels;
				}
			}
		}
	}

	_frame_position += count;
	return count;
}


// static
cstr* AudioDecoder::supportedFileExtensions()
{
//  http://developer.apple.com/library/mac/documentation/MusicAudio/Reference/AudioFileConvertRef/Reference/reference.html#//apple_ref/doc/c_ref/AudioFileTypeID
//	kAudioFileAIFFType	= 'AIFF',
//	kAudioFileAIFCType	= 'AIFC',
//	kAudioFileWAVEType	= 'WAVE',
//	kAudioFileSoundDesigner2Type = 'Sd2f',
//	kAudioFileNextType	= 'NeXT',
//	kAudioFileMP3Type	= 'MPG3',
//	kAudioFileMP2Type	= 'MPG2',
//	kAudioFileMP1Type	= 'MPG1',
//	kAudioFileAC3Type	= 'ac-3',
//	kAudioFileAAC_ADTSType = 'adts',
//	kAudioFileMPEG4Type = 'mp4f',
//	kAudioFileM4AType	= 'm4af',
//	kAudioFileCAFType	= 'caff',
//	kAudioFile3GPType	= '3gpp',
//	kAudioFile3GP2Type	= '3gp2',
//	kAudioFileAMRType	= 'amrf'

	static cstr list[] = {"mp3","m4a","mp2","aiff","wav",NULL};
	return list;
}






