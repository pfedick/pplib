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
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
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

#ifndef _PPL7_INCLUDE_AUDIO
#define _PPL7_INCLUDE_AUDIO
#ifndef _PPL7_INCLUDE
    #ifdef PPL7LIB
        #include "ppl7.h"
    #else
        #include <ppl7.h>
    #endif
#endif

#ifndef PPL7INET_H_
    #ifdef PPL7LIB
		#include "ppl7-inet.h"
    #else
		#include <ppl7-inet.h>
    #endif
#endif


#ifndef LPVOID
#define LPVOID void*
#endif

#include <stdlib.h>
#include <list>


namespace ppl7 {

PPLEXCEPTION(InvalidGenreException,Exception);


//!\brief Struktur zum Speichern eines MP3-Headers
typedef struct tagMPEGHeader{
	ppluint64 start,end;		// Beginn und Ende der Daten
	ppluint64 size,filesize;
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
	ppluint64 mslength;			// Laenge in ms
	ppluint64 length;			// Laenge in s
	ppluint64 samples;
	bool vbr;
} PPL_MPEG_HEADER;

//!\brief Identifizieren von MP3-Dateien und Einlesen der Header
bool IdentMPEG(FileObject &file, PPL_MPEG_HEADER * mpg);


class AudioInfo
{
	public:
		AudioInfo();

		enum AudioFormat {
			WAVE,
			AIFF,
			MP3
		};
		enum ChannelMode {
			STEREO,
			MONO,
			DUAL_CHANNEL,
			JOINT_STEREO
		};

		AudioFormat	Format;
		bool		HaveID3v2Tag;
		bool		IsVBR;
		ppluint32 	ID3v2TagStart;
		ppluint32	FileSize;
		ppluint32	AudioStart;
		ppluint32	AudioEnd;
		ppluint32	AudioSize;
		ppluint32	Samples;
		ppluint8	BytesPerSample;
		ppluint32	Length;		// Length in ms
		ppluint32	Frequency;
		ppluint16	Bitrate;
		ppluint8	Channels;
		ChannelMode	Mode;
};

bool IdentAudioFile(FileObject &file, AudioInfo &info);

class ID3Frame
{
	friend class ID3Tag;
	private:
		char ID[5];
		int Flags;
		size_t Size;
		char *data;
		ID3Frame *nextFrame, *previousFrame;

	public:
		ID3Frame();
		ID3Frame(const String &name);
		~ID3Frame();
};

class ID3Tag
{
	public:
		enum TextEncoding {
			ENC_USASCII,
			ENC_ISO88591,
			ENC_UTF16,
			ENC_UTF8
		};

		enum AudioFormat {
			AF_UNKNOWN=0,
			AF_MP3,
			AF_AIFF
		};
	private:
		String	Filename;
		int		Flags;
		size_t	numFrames;
		int		Size;
		AudioFormat	myAudioFormat;
		ppluint32	PaddingSize, PaddingSpace, MaxPaddingSpace;
		ID3Frame	*firstFrame, *lastFrame;
		int		AddFrame(ID3Frame *Frame);
		void	CopyAndDecodeText(String &s, ID3Frame *frame, int offset) const;
		int	Decode(ID3Frame *frame, int offset, int encoding, String &target) const;
		int SetTextFrameUtf16(const String &framename, const String &text);
		int SetTextFrameISO88591(const String &framename, const String &text);
		int SetTextFrameUtf8(const String &framename, const String &text);

		AudioFormat identAudioFormat(FileObject &File);
		ppluint32 findId3Tag(FileObject &File);

		int SaveMP3();
		int SaveAiff();
		int TrySaveAiffInExistingFile(FileObject &o, ByteArrayPtr &tagV2);
		int CopyAiffToNewFile(FileObject &o, FileObject &n, ByteArrayPtr &tagV2);

	public:
		ID3Tag();
		ID3Tag(const String &File);
		~ID3Tag();

		int load(const String &File);
		int load(FileObject &File);
		void clearTags();
		void clear();
		int save();

		void setPaddingSize(int bytes);
		void setPaddingSpace(int bytes);
		void setMaxPaddingSpace(int bytes);

		void removeFrame(ID3Frame *frame);
		void deleteFrame(ID3Frame *frame);
		ID3Frame	*findFrame(const String &name) const;
		ID3Frame	*findUserDefinedText(const String &description) const;

		void setArtist(const String &artist);
		void setTitle(const String &title);
		void setGenre(const String &genre);
		void setRemixer(const String &remixer);
		void setLabel(const String &label);
		void setComment(const String &comment);
		void setComment(const String &description, const String &comment);
		void setYear(const String &year);
		void setAlbum(const String &album);
		void setTrack(const String &track);
		void setBPM(const String &bpm);
		void setKey(const String &key);
		void setEnergyLevel(const String &energy);
		void setTextFrame(const String &framename, const String &text, TextEncoding enc=ENC_UTF16);
		void setPicture(int type, const ByteArrayPtr &bin, const String &MimeType);

		void generateId3V2Tag(ByteArray &tag);
		void generateId3V1Tag(ByteArray &tag);


		void listFrames(bool hexdump=false) const;
		size_t frameCount() const;

		String getArtist() const;
		String getTitle() const;
		String getGenre() const;
		String getYear() const;
		String getComment() const;
		String getComment(const String &description) const;
		String getRemixer() const;
		String getLabel() const;
		String getAlbum() const;
		String getTrack() const;
		String getBPM() const;
		String getKey() const;
		String getEnergyLevel() const;
		ByteArray getPicture(int type) const;
		void getPicture(int type, ByteArray &bin) const;

		int getPrivateData(ByteArray &bin, const String &identifier) const;
		ByteArrayPtr getPrivateData(const String &identifier) const;


		void removePicture(int type);
};

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

class AudioCD
{
	private:
		void *cdio;
		String myDevice;
		size_t first_track_num;
		size_t i_tracks;
		size_t num_audio_tracks;
		size_t audio_frames;
		size_t last_lsn;
		void countAudioTracks();

	public:
		PPLEXCEPTION(DeviceOpenFailed,Exception);
		PPLEXCEPTION(DeviceNotOpen,Exception);
		PPLEXCEPTION(InvalidAudioTrack,Exception);

		class Toc
		{
			public:
				ppluint8 min,sec,frames;
		};

		class Track
		{
			friend class AudioCD;
			private:
				int _track;
				size_t _start, _end;
				bool _hasPreemphasis, _hasCopyPermit, _isAudioTrack;
				int _channels;

			public:
				Track();
				int track() const;
				size_t start() const;
				Toc start_toc() const;
				size_t end() const;
				size_t size() const;
				size_t seconds() const;
				bool hasPreemphasis() const;
				bool hasCopyPermit() const;
				bool isAudioTrack() const;
				int channels() const;
		};

		AudioCD();
		~AudioCD();

		void openDevice(const String &device=String());
		void closeDevice();
		const String &deviceName() const;

		size_t firstTrack() const;
		size_t lastTrack() const;
		size_t numTotalTracks() const;
		size_t numAudioTracks() const;
		size_t totalAudioFrames() const;
		size_t totalAudioLength() const;
		size_t lastLsn() const;

		AudioCD::Track getTrack(int track);
		bool	isAudioTrack(int track);

		static bool isSupported();
		static std::list<String> getDevices();
		static Toc lsn2toc(size_t lsn);
};


class CDDB
{
	public:
		PPLEXCEPTION(QueryFailed, Exception);
		PPLEXCEPTION(InvalidDiscId, Exception);
		class Track
		{
			public:
				int number;
				int frame_offset;
				int length;
				String Artist;
				String Title;
				String Extra;
		};
		class Disc
		{
			public:
				typedef std::list<Track> TrackList;

				unsigned int	discId;
				String		category;
				String		genre;
				int			length;
				int			year;
				String		Artist;
				String		Title;
				String		Extra;
				TrackList	Tracks;
		};

		typedef std::list<Disc> Matches;

	private:
		ppl7::Curl curl;
		String QueryPath, Server;
		String ClientName, ClientVersion;
		String UserName, Hostname;
		int port;
		String buildUri(const String &cmd);

	public:
		CDDB();
		~CDDB();

		void	setHttpServer(const String &server, int port=80);
		void	setQueryPath(const String &path);
		void	setProxy(const String &hostname, int port);
		void	setClient(const String &name, const String &version);
		void	setUser(const String &username, const String &hostname);

		int		query(AudioCD &cd, Matches &list);
		void	getDisc(unsigned int discId, const String &category, Disc &d);

		static bool isSupported();
		static unsigned int calcDiscId(AudioCD &cd);

};

//!\brief Struktur zum Speichern eines WAVE-Headers
typedef struct tagWAVEHeader {
	ppluint32	datastart;
	ppluint32	numSamples;
	ppluint32	bytes;
	ppluint32	frequency;
	ppluint32	bytespersample;
	ppluint32	bytespersecond;
	ppluint32	seconds;
	ppluint8	channels;
	ppluint8	bitdepth;
} WAVEHEADER;


class AudioDecoder
{
	public:
};

class AudioEncoder
{

};

class AudioDecoder_Wave
{
	private:
		File			myFile;
		FileObject		*ff;
		WAVEHEADER wave;
		size_t position;
		size_t samplesize;
		void readWaveHeader(FileObject &file, WAVEHEADER &header);

	public:
		AudioDecoder_Wave();
		~AudioDecoder_Wave();
		void close();
		bool ident(const String &filename);
		bool ident(FileObject &file);
		void open(const String &filename);
		void open(FileObject &file);
		void getWaveHeader(WAVEHEADER &header) const;
		size_t bitdepth() const;
		size_t bytesPerSample() const;
		void seekSample(size_t sample);
		size_t getSamples(size_t num, void *interleafed);
		size_t getSamples(size_t num, float *left, float *right);
		size_t getSamples(size_t num, void *left, void *right);

};


} // end of namespace ppl7


#endif // _PPL6_INCLUDE_SOUND

