/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:57 $
 * $Id: ppl6-sound.h,v 1.2 2010/02/12 19:43:57 pafe Exp $
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

#ifndef _PPL6_INCLUDE_SOUND
#define _PPL6_INCLUDE_SOUND
#ifndef _PPL6_INCLUDE
    #ifdef PPL6LIB
        #include "ppl6.h"
    #else
        #include <ppl6.h>
    #endif
#endif


#ifndef LPVOID
#define LPVOID void*
#endif

namespace ppl6 {

namespace MP3 {
	//!\brief Namespace für MP3-spezifische Definitionen
	namespace MODE {
		//!\brief Channel-Mode für MP3-Dateien
		enum {
			STEREO=0,
			MONO,
			DUAL_CHANNEL,
			JOINT_STEREO
		};
	}
}

// TODO: Defines umwandeln in Enumerations im Namespace MP3
#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

#define         MPG_MD_LR_LR             0
#define         MPG_MD_LR_I              1
#define         MPG_MD_MS_LR             2
#define         MPG_MD_MS_I              3


#define SAMPLE	pplint16
//!\brief Struktur zum Speichern eines Stereo-Samples
typedef struct tagSTEREOSAMPLE{
    pplint16 left;
    pplint16 right;
} STEREOSAMPLE;


//!\brief Struktur zum Speichern eines WAVE-Headers
typedef struct tagWAVEHeader {
	ppldd	datastart;
	ppldd	numSamples;
	ppldd	bytes;
	ppldd	frequency;
	ppldd	bytespersample;
	ppldd	bytespersecond;
	ppldd	seconds;
	ppldb	channels;
	ppldb	bitdepth;
} PPL_WAVEHEADER;

//!\brief Struktur zum Speichern eines MP3-Headers
typedef struct tagMPEGHeader{
	ppldd start,end;		// Beginn und Ende der Daten
	ppldd size,filesize;
	int version;
    int layer;
    int error_protection;
    int bitrate_index;
	int bitrate;
    int frequency_index;
	int frequency;
    int padding;
    int extension;
    int mode;
    int mode_ext;
    int copyright;
    int original;
    int emphasis;
    int stereo;
	int	framesize;
	int frames;
	ppldd mslength;			// Laenge in ms
	ppldd length;			// Laenge in s
	ppldd samples;
	bool vbr;
} PPL_MPEG_HEADER;

//!\brief Struktur zum Speichern des Fortschritts bei einem MP3-Encode-Vorgang
typedef struct tagSOUNDPROGRESS{
	void	*data;
	//ppldd	tickspersecond;
	double	timestarted;
	double	timeend;
	double	now;
	double	past;
	float	percent;
	float	faktor;
	double	eta;
	ppld64	position;
	ppld64	bytes;
	ppld64	position_thisfile;
	ppld64	bytes_thisfile;
	PPL_WAVEHEADER *wav;
	PPL_MPEG_HEADER *mpg;
} PPL_SOUNDPROGRESS;


const char *GetID3GenreName(int id);

class CMP3Decode;

class CMP3Encode
//!\brief Klasse zum Encoden von MP3-Files
{
	private:
		void * gfp;
		PPL_WAVEHEADER firstwave;
		PPL_SOUNDPROGRESS progress;
		CFileObject *out;
		char			*readcache;
		int				mp3bufsize;
		int				encoderdelay;
		unsigned char	*mp3buf;

		ppldd	samples;

		void (*ProgressFunc) (PPL_SOUNDPROGRESS *prog);

		bool have_firstwave;
		bool started;
		bool bStopEncode;

		void WriteEncodedBytes(char *buffer, ppldd bytes);

		CThread *WatchThread;
		int HandleWatchThread();

	public:
		CMP3Encode();
		~CMP3Encode();


		void SetBitrate(int kbps=192);
		void SetVBR(int min=32, int max=320, int quality=2);
		void SetCBR(int kbps=192, int quality=2);
		void SetABR(int kbps=192, int quality=2);
		void SetQuality(int quality=2);			// 1=best, 10=lowest
		void SetMode(int mode=0);				// 0=Stereo
		void SetLowpassFreq(int freq=-1);		// -1=Disabled
		void SetHighpassFreq(int freq=-1);		// -1=Disabled

		void SetProgressFunction(void (*Progress) (PPL_SOUNDPROGRESS *prog), void *data);
		void SetEncoderDelay(int milliseconds);
		void SetWatchThread(CThread *t);

		int StartEncode(CFileObject *output=NULL);
		int StartEncode(int frequency, int channels);
		int EncodeFile(CFileObject *file);
		int EncodeFile(CMP3Decode *file, CFileObject *source);
		int EncodeBuffer(SAMPLE *left, SAMPLE *right, int num, void *mp3buf, int buffersize);
		int EncodeBuffer(STEREOSAMPLE *buffer, int num, void *mp3buf, int buffersize);
		int FlushBuffer(void *mp3buf, int buffersize);
		void StopEncode();

		int FinishEncode();

		int ReadWaveHeader(CFileObject *ff, PPL_WAVEHEADER *wav);

		const char *GetLameVersion();
		const char *GetPSYVersion();


};


//!\brief Identifizieren von MP3-Dateien und Einlesen der Header
bool IdentMPEG(CFileObject * file, PPL_MPEG_HEADER * mpg);


class CMP3Decode
{
//!\brief Klasse zum Dekodieren von MP3-Files
	private:
		PPL_MPEG_HEADER	mpg;
		CMutex	mutex;
		CFileObject	*ff;
		void		*mad;
		ppluint32	FrameCount;
		ppluint8	*readbuffer;
		SAMPLE		*out_left, *out_right;
		int			out_offset, out_samples, out_size;

		bool		closefile;
		bool		isRunning;
		bool		loop;

		int FillDecodeBuffer();

	public:
		CMP3Decode();
		~CMP3Decode();

		bool DoFilter;

		void Clear();
		int LoadFile(char *filename);
		int LoadFile(CFileObject *file);
		int GetMpegHeader(PPL_MPEG_HEADER *mpg);
		int Start();
		int Stop();
		int Decode(int num, SAMPLE *left, SAMPLE *right);
			// Falls der Returnwert < num ist, sind keine weiteren
			// Daten mehr vorhanden und die restlichen Samples sind 0.
};



class CID3Frame
{
	friend class CID3Tag;
	private:
		char ID[5];
		int Flags;
		size_t Size;
		char *data;
		CID3Frame *nextFrame, *previousFrame;

	public:
		CID3Frame();
		CID3Frame(const char *name);
		~CID3Frame();
};

class CID3Tag
{
	private:
		CString	Filename;
		CLog	*Log;
		int		Flags;
		int		Size;
		ppluint32	PaddingSize, PaddingSpace, MaxPaddingSpace;
		CID3Frame	*firstFrame, *lastFrame;
		int		AddFrame(CID3Frame *Frame);
		void	CopyAndDecodeText(CString &s, CID3Frame *frame, int offset) const;
		void	Decode(CID3Frame *frame, int offset, int encoding, CString &target) const;

	public:
		CID3Tag();
		CID3Tag(const char *filename);

		void SetLogfile(CLog *log);

		int Load(const char *file);
		int Load(const CString &File);
		int Load(CFileObject *File);
		void ClearTags();

		int Save(bool writev1=true, bool writev2=true);

		void SetPaddingSize(int bytes);
		void SetPaddingSpace(int bytes);
		void SetMaxPaddingSpace(int bytes);

		int SetArtist(const CString &artist);
		int SetTitle(const CString &title);
		int SetGenre(const CString &genre);
		int SetRemixer(const CString &remixer);
		int SetLabel(const CString &label);
		int SetComment(const CString &comment);
		int SetComment(const CString &description, const CString &comment);
		int SetYear(const CString &year);
		int SetAlbum(const CString &album);
		int SetTrack(const CString &track);
		int SetBPM(const CString &bpm);
		int SetTextFrame(const char *framename, const CString &text);
		int			RemoveFrame(CID3Frame *frame);
		int			DeleteFrame(CID3Frame *frame);
		CID3Frame	*FindFrame(const char *name) const;
		CID3Frame	*FindFrame(const CString &name) const;
		CID3Frame	*FindUserDefinedText(const char *description) const;
		CID3Frame	*FindUserDefinedText(const CString &description) const;
		char *MakeIdV1Tag();
		void Clear();
		void ListFrames(int hexdump=0) const;

		CString GetArtist() const;
		CString GetTitle() const;
		CString GetGenre() const;
		CString GetYear() const;
		CString GetComment() const;
		CString GetComment(const CString &description) const;
		CString GetRemixer() const;
		CString GetLabel() const;
		CString GetAlbum() const;
		CString GetTrack() const;
		CString GetBPM() const;

		~CID3Tag();
};



} // end of namespace ppl


#endif // _PPL6_INCLUDE_SOUND

