/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
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

#ifndef _PPL7_INCLUDE_SOUND
#define _PPL7_INCLUDE_SOUND
#ifndef _PPL7_INCLUDE
    #ifdef PPL7LIB
        #include "ppl7.h"
    #else
        #include <ppl7.h>
    #endif
#endif


#ifndef LPVOID
#define LPVOID void*
#endif

namespace ppl7 {

class Icecast
{
	private:
		void	*shout;
		bool    bconnected;
	public:
		Icecast();
		~Icecast();
		String	getVersion(int *major=NULL, int *minor=NULL, int *patch=NULL) const;
		String version() const;
		bool	connected();
		void	setConnection(const String &host, int port, const String &password);
		String host() const;
		int		port() const;
		String	password() const;
		void	setMount(const String &mount);
		String mount() const;
		void	setName(const String &name);
		String name() const;
		void	setUrl(const String &url);
		String url() const;
		void	setGenre(const String &genre);
		String genre() const;
		void	setUser(const String &user);
		String user() const;
		void	setAgent(const String &agent);
		String agent() const;
		void	setDescription(const String &description);
		String description() const;
		void	setDumpfile(const String &file);
		String dumpfile() const;
		void	setAudioInfo(const String &name, const String &value);
		String audioInfo(const String &name) const;
		void	setPublic(bool makepublic);
		void	setFormatMP3();
		void	setFormatOGG();
		void	setNonBlocking(bool flag);

		void	connect();
		void	disconnect();

		void	send(const void *buffer, size_t bytes);
		int		delay();
		void	sync();

		void	sendMetadata(const String &name, const String &value);
		void	setTitle(const String &title);

};



/*
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

void MpegHeader2Array(ppl6::CAssocArray &a, const PPL_MPEG_HEADER *mpg);
void PrintMpegHeader(const PPL_MPEG_HEADER *mpg);

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
		int EncodeFile(CMP3Decode *file);
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


// Basisklasse
class CMP3Decode
{
	private:
		CMP3Decode	*decoder;
	public:
		CMP3Decode();
		virtual ~CMP3Decode();
		int Init(const char *engine=NULL);
		virtual void Clear();
		virtual int LoadFile(const String &filename);
		virtual int LoadFile(CFileObject &file);
		virtual int GetMpegHeader(PPL_MPEG_HEADER *mpg);
		virtual int Start();
		virtual int Stop();
		virtual int Decode(int num, SAMPLE *left, SAMPLE *right);
};

class CMP3DecodeLibMad : public CMP3Decode
{
//!\brief Klasse zum Dekodieren von MP3-Files
	private:
		PPL_MPEG_HEADER	mpg;
		CMutex	mutex;
		CFileObject	*ff;
		CFile		*file;
		void		*mad;
		ppluint32	FrameCount;
		ppluint8	*readbuffer;
		SAMPLE		*out_left, *out_right;
		int			out_offset, out_samples, out_size;
		bool		isRunning;
		bool		loop;

		int FillDecodeBuffer();

	public:
		CMP3DecodeLibMad();
		~CMP3DecodeLibMad();

		bool DoFilter;

		virtual void Clear();
		virtual int LoadFile(const String &filename);
		virtual int LoadFile(CFileObject &file);
		virtual int GetMpegHeader(PPL_MPEG_HEADER *mpg);
		virtual int Start();
		virtual int Stop();
		virtual int Decode(int num, SAMPLE *left, SAMPLE *right);
			// Falls der Returnwert < num ist, sind keine weiteren
			// Daten mehr vorhanden und die restlichen Samples sind 0.
};

class CMP3DecodeLibMpg123 : public CMP3Decode
{
	private:
		PPL_MPEG_HEADER	mpg;
		CFileObject	*ff;
		CFile		*file;
		void		*hh;
		ppluint8	*readbuffer;
		ppluint8	*outbuffer;
		size_t		out_offset, out_size;

		bool isInitialized;
		bool		isRunning;
		bool		needInput;

		int FillDecodeBuffer();

	public:
		CMP3DecodeLibMpg123();
		~CMP3DecodeLibMpg123();
		virtual void Clear();
		virtual int LoadFile(const String &filename);
		virtual int LoadFile(CFileObject &file);
		virtual int GetMpegHeader(PPL_MPEG_HEADER *mpg);
		virtual int Start();
		virtual int Stop();
		virtual int Decode(int num, SAMPLE *left, SAMPLE *right);
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
		String	Filename;
		CLog	*Log;
		int		Flags;
		int		Size;
		ppluint32	PaddingSize, PaddingSpace, MaxPaddingSpace;
		CID3Frame	*firstFrame, *lastFrame;
		int		AddFrame(CID3Frame *Frame);
		void	CopyAndDecodeText(String &s, CID3Frame *frame, int offset) const;
		int	Decode(CID3Frame *frame, int offset, int encoding, String &target) const;

	public:
		CID3Tag();
		CID3Tag(const char *filename);

		void SetLogfile(CLog *log);

		int Load(const char *file);
		int Load(const String &File);
		int Load(CFileObject *File);
		void ClearTags();

		int Save(bool writev1=true, bool writev2=true);

		void SetPaddingSize(int bytes);
		void SetPaddingSpace(int bytes);
		void SetMaxPaddingSpace(int bytes);

		int SetArtist(const String &artist);
		int SetTitle(const String &title);
		int SetGenre(const String &genre);
		int SetRemixer(const String &remixer);
		int SetLabel(const String &label);
		int SetComment(const String &comment);
		int SetComment(const String &description, const String &comment);
		int SetYear(const String &year);
		int SetAlbum(const String &album);
		int SetTrack(const String &track);
		int SetBPM(const String &bpm);
		int SetTextFrame(const char *framename, const String &text);
		int			RemoveFrame(CID3Frame *frame);
		int			DeleteFrame(CID3Frame *frame);
		CID3Frame	*FindFrame(const char *name) const;
		CID3Frame	*FindFrame(const String &name) const;
		CID3Frame	*FindUserDefinedText(const char *description) const;
		CID3Frame	*FindUserDefinedText(const String &description) const;
		char *MakeIdV1Tag();
		void Clear();
		void ListFrames(int hexdump=0) const;

		String GetArtist() const;
		String GetTitle() const;
		String GetGenre() const;
		String GetYear() const;
		String GetComment() const;
		String GetComment(const String &description) const;
		String GetRemixer() const;
		String GetLabel() const;
		String GetAlbum() const;
		String GetTrack() const;
		String GetBPM() const;
		int GetPicture(int type, CBinary &bin) const;
		int SetPicture(int type, const CBinary &bin, const String &MimeType);
		void RemovePicture(int type);
		~CID3Tag();
};

*/



} // end of namespace ppl7


#endif // _PPL6_INCLUDE_SOUND

