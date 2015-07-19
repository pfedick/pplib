/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include "prolog.h"
#include <string.h>
#include <limits.h>
#include "ppl6.h"
#include "ppl6-sound.h"


#undef DEBUG

#ifdef DEBUG
	#define debugprinterror() ppl6::PushError(); { \
		ppl6::CString debugerror, debugdate; \
		ppl6::Error2String(debugerror); \
		ppl6::MkDate(debugdate,"%d.%m.%Y-%H:%M:%S",ppl6::GetTime()); \
		printf ("%s.%i [%s:%i] ",(const char*)debugdate,(int)(ppl6::GetMilliSeconds()%1000),__FILE__,__LINE__); \
		printf ("%s",(const char*)debugerror); } ppl6::PopError();

	#define debugprintf(format, ...) ppl6::PushError(); { \
		ppl6::CString debugdate; \
		ppl6::MkDate(debugdate,"%d.%m.%Y-%H:%M:%S",ppl6::GetTime()); \
		printf ("%s.%i [%s:%i] ",(const char*)debugdate,(int)(ppl6::GetMilliSeconds()%1000),__FILE__,__LINE__); \
		printf (format, ##__VA_ARGS__); } ppl6::PopError();
#else
	#define debugprintf(format, ...) {}
	#define debugprinterror() {}
#endif




#ifdef HAVE_LIBMAD
#include <mad.h>
#endif

namespace ppl6 {


#ifdef HAVE_LIBMAD
#define READBUFFER	131072
#define OUTSAMPLES	2048

typedef struct tagMadStruct{
	struct mad_stream	Stream;
	struct mad_frame	Frame;
	struct mad_synth	Synth;
	mad_timer_t			Timer;
} MADSTRUCT;

//#define INCLUDEDEBUG

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

static inline signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/****************************************************************************
 * Converts a sample from libmad's fixed point number format to a signed	*
 * short (16 bits).															*
 ****************************************************************************/
static SAMPLE MadFixedToSshort(mad_fixed_t Fixed)
{
	/* A fixed point number is formed of the following bit pattern:
	 *
	 * SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
	 * MSB                          LSB
	 * S ==> Sign (0 is positive, 1 is negative)
	 * W ==> Whole part bits
	 * F ==> Fractional part bits
	 *
	 * This pattern contains MAD_F_FRACBITS fractional bits, one
	 * should alway use this macro when working on the bits of a fixed
	 * point number. It is not guaranteed to be constant over the
	 * different platforms supported by libmad.
	 *
	 * The signed short value is formed, after clipping, by the least
	 * significant whole part bit, followed by the 15 most significant
	 * fractional part bits. Warning: this is a quick and dirty way to
	 * compute the 16-bit number, madplay includes much better
	 * algorithms.
	 */

	/* Clipping */
	if(Fixed>=MAD_F_ONE)
		return(SHRT_MAX);
	if(Fixed<=-MAD_F_ONE)
		return(-SHRT_MAX);

	/* Conversion. */
	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((SAMPLE)Fixed);
}

#ifdef INCLUDEDEBUG
/****************************************************************************
 * Print human readable informations about an audio MPEG frame.				*
 ****************************************************************************/
static int PrintFrameInfo(FILE *fp, struct mad_header *Header)
{
	const char	*Layer,
				*Mode,
				*Emphasis;

	/* Convert the layer number to it's printed representation. */
	switch(Header->layer)
	{
		case MAD_LAYER_I:
			Layer="I";
			break;
		case MAD_LAYER_II:
			Layer="II";
			break;
		case MAD_LAYER_III:
			Layer="III";
			break;
		default:
			Layer="(unexpected layer value)";
			break;
	}

	/* Convert the audio mode to it's printed representation. */
	switch(Header->mode)
	{
		case MAD_MODE_SINGLE_CHANNEL:
			Mode="single channel";
			break;
		case MAD_MODE_DUAL_CHANNEL:
			Mode="dual channel";
			break;
		case MAD_MODE_JOINT_STEREO:
			Mode="joint (MS/intensity) stereo";
			break;
		case MAD_MODE_STEREO:
			Mode="normal LR stereo";
			break;
		default:
			Mode="(unexpected mode value)";
			break;
	}

	/* Convert the emphasis to it's printed representation. Note that
	 * the MAD_EMPHASIS_RESERVED enumeration value appeared in libmad
	 * version 0.15.0b.
	 */
	switch(Header->emphasis)
	{
		case MAD_EMPHASIS_NONE:
			Emphasis="no";
			break;
		case MAD_EMPHASIS_50_15_US:
			Emphasis="50/15 us";
			break;
		case MAD_EMPHASIS_CCITT_J_17:
			Emphasis="CCITT J.17";
			break;
#if (MAD_VERSION_MAJOR>=1) || \
	((MAD_VERSION_MAJOR==0) && (MAD_VERSION_MINOR>=15))
		case MAD_EMPHASIS_RESERVED:
			Emphasis="reserved(!)";
			break;
#endif
		default:
			Emphasis="(unexpected emphasis value)";
			break;
	}


	fprintf(fp,"PPL6: %lu kb/s audio MPEG layer %s stream %s CRC, "
			"%s with %s emphasis at %d Hz sample rate\n",
			Header->bitrate,Layer,
			Header->flags&MAD_FLAG_PROTECTION?"with":"without",
			Mode,Emphasis,Header->samplerate);
	return(ferror(fp));
}
#endif

// TODO: Ist dies Multithreading sicher?
static mad_fixed_t	Filter[32];
/****************************************************************************
 * Applies a frequency-domain filter to audio data in the subband-domain.	*
 ****************************************************************************/
static void ApplyFilter(struct mad_frame *Frame)
{
	int	Channel,
		Sample,
		Samples,
		SubBand;

	/* There is two application loops, each optimized for the number
	 * of audio channels to process. The first alternative is for
	 * two-channel frames, the second is for mono-audio.
	 */
	Samples=MAD_NSBSAMPLES(&Frame->header);
	if(Frame->header.mode!=MAD_MODE_SINGLE_CHANNEL)
		for(Channel=0;Channel<2;Channel++)
			for(Sample=0;Sample<Samples;Sample++)
				for(SubBand=0;SubBand<32;SubBand++)
					Frame->sbsample[Channel][Sample][SubBand]=
						mad_f_mul(Frame->sbsample[Channel][Sample][SubBand],
								  Filter[SubBand]);
	else
		for(Sample=0;Sample<Samples;Sample++)
			for(SubBand=0;SubBand<32;SubBand++)
				Frame->sbsample[0][Sample][SubBand]=
					mad_f_mul(Frame->sbsample[0][Sample][SubBand],
							  Filter[SubBand]);
}


#ifdef INCLUDEDEBUG
/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */

static enum mad_flow output(void *data,
             struct mad_header const *header,
             struct mad_pcm *pcm)
{
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;
	//CMP3DecodeLibMad *me=(CMP3DecodeLibMad *) data;

	/* pcm->samplerate contains the sampling frequency */
	nchannels = pcm->channels;
	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
	right_ch  = pcm->samples[1];

	while (nsamples--) {
		signed int sample;
		/* output sample(s) in 16-bit signed little-endian PCM */
		sample = scale(*left_ch++);
		putchar((sample >> 0) & 0xff);
		putchar((sample >> 8) & 0xff);
		if (nchannels == 2) {
			sample = scale(*right_ch++);
			putchar((sample >> 0) & 0xff);
			putchar((sample >> 8) & 0xff);
		}
	}
	return MAD_FLOW_CONTINUE;
}
#endif


// ######################################################################
// ### CMP3DecodeLibMad-Klasse                                           ###
// ######################################################################

#endif

/*!\class CMP3DecodeLibMad
 * \ingroup PPLGroupSound
 */


CMP3DecodeLibMad::CMP3DecodeLibMad()
{
#ifdef HAVE_LIBMAD
	MADSTRUCT *m=(MADSTRUCT*)malloc(sizeof(MADSTRUCT));
	mad=m;
	ff=NULL;
	file=NULL;
	readbuffer=NULL;
	isRunning=false;
	loop=false;
	DoFilter=false;
	out_size=OUTSAMPLES;
	out_offset=0;
	out_samples=0;
	out_left=NULL;
	out_right=NULL;
#endif

}

CMP3DecodeLibMad::~CMP3DecodeLibMad()
{
#ifdef HAVE_LIBMAD
	PushError();
	Clear();
	free(mad);
	free(readbuffer);
	free(out_left);
	free(out_right);
	PopError();
#endif
}

void CMP3DecodeLibMad::Clear()
{
#ifdef HAVE_LIBMAD
	Stop();
	delete file;
	file=NULL;
	ff=NULL;
#endif
}

int CMP3DecodeLibMad::LoadFile(const CString &filename)
{
#ifdef HAVE_LIBMAD
	Clear();
	file=new CFile();
	if (!file->Open(filename,"rb")) {
		delete file;
		return 0;
	}
	int ret=LoadFile(*file);
	if (!ret) {
		PushError();
		delete file;
		PopError();
	}
	return ret;
#else
	ppl6::SetError(555,"libmad");
	return 0;
#endif
}

int CMP3DecodeLibMad::LoadFile(CFileObject &file)
{
#ifdef HAVE_LIBMAD
	if (!IdentMPEG(&file,&mpg)) return 0;
	ff=&file;
	return 1;
#else
	ppl6::SetError(555,"libmad");
	return 0;
#endif
}

int CMP3DecodeLibMad::GetMpegHeader(PPL_MPEG_HEADER *mpg)
{
#ifdef HAVE_LIBMAD
	if (!mpg) {
		SetError(194,"PPL_MPEG_HEADER *mpg");
		return 0;
	}
	if (!ff) {
		SetError(239);
		return 0;
	}
	memcpy(mpg,&this->mpg,sizeof(PPL_MPEG_HEADER));
	return 1;
#else
	ppl6::SetError(555,"libmad");
	return 0;
#endif
}

int CMP3DecodeLibMad::Start()
{
#ifdef HAVE_LIBMAD
	Stop();
	if (!ff) {
		SetError(239);
		return 0;
	}
	// Buffer Reservieren, falls noch nicht vorhanden
	if (!readbuffer) {
		readbuffer=(ppluint8*)malloc(READBUFFER+MAD_BUFFER_GUARD+16);
		if (!readbuffer) {
			SetError(2,"readbuffer");
			return 0;
		}
	}
	if (!out_left) {
		out_left=(SAMPLE*)malloc(sizeof(SAMPLE)*out_size);
		if (!out_left) {
			SetError(2,"out_left");
			return 0;
		}
	}
	if (!out_right) {
		out_right=(SAMPLE*)malloc(sizeof(SAMPLE)*out_size);
		if (!out_right) {
			SetError(2,"out_right");
			return 0;
		}
	}
	out_offset=0;
	out_samples=0;
	ff->Seek(0);
	MADSTRUCT *m=(MADSTRUCT*)mad;
	/* First the structures used by libmad must be initialized. */
	mad_stream_init(&m->Stream);
	mad_frame_init(&m->Frame);
	mad_synth_init(&m->Synth);
	mad_timer_reset(&m->Timer);
	isRunning=true;
	FrameCount=0;
	return 1;
#else
	ppl6::SetError(555,"libmad");
	return 0;
#endif
}


int CMP3DecodeLibMad::Stop()
{
#ifdef HAVE_LIBMAD
	mutex.Lock();
	if(!isRunning) {
		mutex.Unlock();
		return 1;
	}
	MADSTRUCT *m=(MADSTRUCT*)mad;
	isRunning=false;
	mad_synth_finish(&m->Synth);
	mad_frame_finish(&m->Frame);
	mad_stream_finish(&m->Stream);
	mutex.Unlock();
	return 1;
#else
	ppl6::SetError(555,"libmad");
	return 0;
#endif
}

int CMP3DecodeLibMad::Decode(int samples, SAMPLE *left, SAMPLE *right)
{
#ifdef HAVE_LIBMAD
	debugprintf("CMP3DecodeLibMad::Decode angefordert=%i, noch im buffer=%i\n",samples,out_samples);

	if (!samples) {
		SetError(258);
		return 0;
	}
	if (!left) {
		SetError(194,"SAMPLE *left");
		return 0;
	}
	if (!right) {
		SetError(194,"SAMPLE *right");
		return 0;
	}
	//int ret=0;
	mutex.Lock();
	if (!isRunning) {
		SetError(257);
		mutex.Unlock();
		return 0;
	}

	int rest=samples;
	int filled=0;
	//int copy=0;
	// Falls noch Samples im DecodeBuffer sind, kopieren wir diese
	// zuerst
	if (out_samples) {
		if (rest>out_samples) {
			debugprintf("rest>out_samples, kopiere %i samples zum Anforderer\n",out_samples);
			memcpy(left,out_left+out_offset,out_samples*sizeof(SAMPLE));
			memcpy(right,out_right+out_offset,out_samples*sizeof(SAMPLE));
			left+=out_samples;
			right+=out_samples;
			rest-=out_samples;
			filled=out_samples;
			out_samples=0;
			debugprintf("rest=%i, out_samples=%i, filled=%i\n",rest,out_samples,filled);
		} else {
			debugprintf("rest<out_samples, kopiere %i samples zum Anforderer\n",out_samples);
			memcpy(left,out_left+out_offset,rest*sizeof(SAMPLE));
			memcpy(right,out_right+out_offset,rest*sizeof(SAMPLE));
			out_samples-=rest;
			out_offset+=rest;
			mutex.Unlock();
			debugprintf("rest=%i, out_samples=%i, filled=%i, beende CMP3DecodeLibMad::Decode\n",0,out_samples,rest);
			return rest;
		}
	}

	// Dann lesen wir solange neue Frames, bis alle Samples gef�llt
	// sind
	while (rest) {
		debugprintf("Decode-Schleife: rest=%i, out_samples=%i, filled=%i\n",rest,out_samples,filled);
		int num=FillDecodeBuffer();
		debugprintf("FillDecodeBuffer hat %i samples geliefert\n",num);
		if (num<=0) break;
		if (num<=rest) {
			debugprintf("num<=rest, Kopiere %i Samples\n",num);
			memcpy(left,out_left,num*sizeof(SAMPLE));
			memcpy(right,out_right,num*sizeof(SAMPLE));
			out_offset=0;
			out_samples=0;
			left+=num;
			right+=num;
			filled+=num;
			rest-=num;
			debugprintf("rest=%i, out_samples=%i, filled=%i\n",rest,out_samples,filled);
		} else {
			debugprintf("num>rest, Kopiere %i Samples\n",rest);
			memcpy(left,out_left,rest*sizeof(SAMPLE));
			memcpy(right,out_right,rest*sizeof(SAMPLE));
			out_offset=rest;
			out_samples=num-rest;
			left+=rest;
			right+=rest;
			filled+=rest;
			rest=0;
			debugprintf("rest=%i, out_samples=%i, filled=%i\n",rest,out_samples,filled);
		}
	}
	debugprintf("Dekodier-Schleife beendet\n");
	if (filled<samples) {
		rest=samples-filled;
		debugprintf("Fülle Rest mit 0 auf: %i Samples\n",rest);
		bzero(left,rest*sizeof(SAMPLE));
		bzero(right,rest*sizeof(SAMPLE));
	}

	mutex.Unlock();
	debugprintf("Beende CMP3DecodeLibMad::Decode, %i Samples angefordert, %i Samples geliefert\n",samples,filled);
	return filled;
#else
	ppl6::SetError(555,"libmad");
	return 0;
#endif
}

int CMP3DecodeLibMad::FillDecodeBuffer()
{
#ifdef HAVE_LIBMAD
	MADSTRUCT *m=(MADSTRUCT*)mad;
	ppluint8	*GuardPtr=NULL;
	//int			Status=0;
	debugprintf("CMP3DecodeLibMad::FillDecodeBuffer\n");

	while (1) {	// Hauptschleife
		debugprintf("Hauptschleife beginn\n");
		/* The input bucket must be filled if it becomes empty or if
		 * it's the first execution of the loop.
		 */
		if(m->Stream.buffer==NULL || m->Stream.error==MAD_ERROR_BUFLEN) {
			ppluint32		ReadSize, Remaining;
			unsigned char	*ReadStart;

			debugprintf("Buffer ist leer, oder Schleife wird erstmalig durchlaufen\n");

			/* {2} libmad may not consume all bytes of the input
			 * buffer. If the last frame in the buffer is not wholly
			 * contained by it, then that frame's start is pointed by
			 * the next_frame member of the Stream structure. This
			 * common situation occurs when mad_frame_decode() fails,
			 * sets the stream error code to MAD_ERROR_BUFLEN, and
			 * sets the next_frame pointer to a non NULL value. (See
			 * also the comment marked {4} bellow.)
			 *
			 * When this occurs, the remaining unused bytes must be
			 * put back at the beginning of the buffer and taken in
			 * account before refilling the buffer. This means that
			 * the input buffer must be large enough to hold a whole
			 * frame at the highest observable bit-rate (currently 448
			 * kb/s). XXX=XXX Is 2016 bytes the size of the largest
			 * frame? (448000*(1152/32000))/8
			 */
			if(m->Stream.next_frame!=NULL) {
				Remaining=(ppldd)(m->Stream.bufend-m->Stream.next_frame);
				memmove(readbuffer,m->Stream.next_frame,Remaining);
				ReadStart=readbuffer+Remaining;
				ReadSize=READBUFFER-Remaining;
			} else {
				ReadSize=READBUFFER;
				ReadStart=readbuffer;
				Remaining=0;
			}

			/* Fill-in the buffer. If an error occurs print a message
			 * and leave the decoding loop. If the end of stream is
			 * reached we also leave the loop but the return status is
			 * left untouched.
			 */

			debugprintf("Lese %u Bytes von Fileposition %llu, in Buffer-Position %u\n",ReadSize,ff->Ftell(),(ReadStart-readbuffer));
			ReadSize=ff->Read(ReadStart,ReadSize);
			debugprintf("Tatsächlich gelesen: %u Bytes\n",ReadSize);
			if (!ReadSize) {
				break;
			}
			/* {3} When decoding the last frame of a file, it must be
			 * followed by MAD_BUFFER_GUARD zero bytes if one wants to
			 * decode that last frame. When the end of file is
			 * detected we append that quantity of bytes at the end of
			 * the available data. Note that the buffer can't overflow
			 * as the guard size was allocated but not used the the
			 * buffer management code. (See also the comment marked
			 * {1}.)
			 *
			 * In a message to the mad-dev mailing list on May 29th,
			 * 2001, Rob Leslie explains the guard zone as follows:
			 *
			 *    "The reason for MAD_BUFFER_GUARD has to do with the
			 *    way decoding is performed. In Layer III, Huffman
			 *    decoding may inadvertently read a few bytes beyond
			 *    the end of the buffer in the case of certain invalid
			 *    input. This is not detected until after the fact. To
			 *    prevent this from causing problems, and also to
			 *    ensure the next frame's main_data_begin pointer is
			 *    always accessible, MAD requires MAD_BUFFER_GUARD
			 *    (currently 8) bytes to be present in the buffer past
			 *    the end of the current frame in order to decode the
			 *    frame."
			 */
			if(ff->Eof())
			{
				GuardPtr=ReadStart+ReadSize;
				memset(GuardPtr,0,MAD_BUFFER_GUARD);
				ReadSize+=MAD_BUFFER_GUARD;
			}

			/* Pipe the new buffer content to libmad's stream decoder
             * facility.
			 */
			mad_stream_buffer(&m->Stream,readbuffer,ReadSize+Remaining);
			m->Stream.error=MAD_ERROR_NONE;

		}	// EOF ReadBuffer fuellen

		/* Decode the next MPEG frame. The streams is read from the
		 * buffer, its constituents are break down and stored the the
		 * Frame structure, ready for examination/alteration or PCM
		 * synthesis. Decoding options are carried in the Frame
		 * structure from the Stream structure.
		 *
		 * Error handling: mad_frame_decode() returns a non zero value
		 * when an error occurs. The error condition can be checked in
		 * the error member of the Stream structure. A mad error is
		 * recoverable or fatal, the error status is checked with the
		 * MAD_RECOVERABLE macro.
		 *
		 * {4} When a fatal error is encountered all decoding
		 * activities shall be stopped, except when a MAD_ERROR_BUFLEN
		 * is signaled. This condition means that the
		 * mad_frame_decode() function needs more input to complete
		 * its work. One shall refill the buffer and repeat the
		 * mad_frame_decode() call. Some bytes may be left unused at
		 * the end of the buffer if those bytes forms an incomplete
		 * frame. Before refilling, the remaining bytes must be moved
		 * to the beginning of the buffer and used for input for the
		 * next mad_frame_decode() invocation. (See the comments
		 * marked {2} earlier for more details.)
		 *
		 * Recoverable errors are caused by malformed bit-streams, in
		 * this case one can call again mad_frame_decode() in order to
		 * skip the faulty part and re-sync to the next frame.
		 */
		if(mad_frame_decode(&m->Frame,&m->Stream)) {
			if(MAD_RECOVERABLE(m->Stream.error)) {
				/* Do not print a message if the error is a loss of
				 * synchronization and this loss is due to the end of
				 * stream guard bytes. (See the comments marked {3}
				 * supra for more informations about guard bytes.)
				 */
				/*
				if(m->Stream.error!=MAD_ERROR_LOSTSYNC ||
				   m->Stream.this_frame!=GuardPtr) {
					fprintf(stderr,"%s: recoverable frame level error (%s)\n",
							ProgName,MadErrorString(&Stream));
					fflush(stderr);
				}
				*/
				continue;
			} else {
				if(m->Stream.error==MAD_ERROR_BUFLEN) {
					continue;
				} else {
					/*
					fprintf(stderr,"%s: unrecoverable frame level error (%s).\n",
							ProgName,MadErrorString(&Stream));
					*/
					//Status=1;
					break;
				}
			}
		}	// EOF mad_frame_decode

		/* The characteristics of the stream's first frame is printed
		 * on stderr. The first frame is representative of the entire
		 * stream.
		 */
		/*
		if(FrameCount==0) {
			if(PrintFrameInfo(stderr,&m->Frame.header))
			{
				Status=1;
				break;
			}
		}
		*/

		/* Accounting. The computed frame duration is in the frame
		 * header structure. It is expressed as a fixed point number
		 * whole data type is mad_timer_t. It is different from the
		 * samples fixed point format and unlike it, it can't directly
		 * be added or subtracted. The timer module provides several
		 * functions to operate on such numbers. Be careful there, as
		 * some functions of libmad's timer module receive some of
		 * their mad_timer_t arguments by value!
		 */
		FrameCount++;
		mad_timer_add(&m->Timer,m->Frame.header.duration);

		/* Between the frame decoding and samples synthesis we can
		 * perform some operations on the audio data. We do this only
		 * if some processing was required. Detailed explanations are
		 * given in the ApplyFilter() function.
		 */
		if(DoFilter) ApplyFilter(&m->Frame);

		/* Once decoded the frame is synthesized to PCM samples. No errors
		 * are reported by mad_synth_frame();
		 */
		mad_synth_frame(&m->Synth,&m->Frame);

		/* Synthesized samples must be converted from libmad's fixed
		 * point number to the consumer format. Here we use unsigned
		 * 16 bit big endian integers on two channels. Integer samples
		 * are temporarily stored in a buffer that is flushed when
		 * full.
		 */
		mad_pcm *pcm=&m->Synth.pcm;
		for(int i=0;i<pcm->length;i++) {
			signed short	Sample;

			/* Left channel */
			Sample=MadFixedToSshort(pcm->samples[0][i]);
			out_left[i]=Sample;

			/* Right channel. If the decoded stream is monophonic then
			 * the right output channel is the same as the left one.
			 */
			if(MAD_NCHANNELS(&m->Frame.header)==2)
				Sample=MadFixedToSshort(pcm->samples[1][i]);
			out_right[i]=Sample;
		}
		out_offset=0;
		out_samples=pcm->length;
		return pcm->length;
	}	// Hauptschleife
	return -1;
#else
	ppl6::SetError(555,"libmad");
	return -1;
#endif
}


}	// EOF namespace ppl6

