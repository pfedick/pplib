/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#ifndef _PPLIB_INCLUDE_AUDIO
#define _PPLIB_INCLUDE_AUDIO
#include <pplib.h>

#include <pplib-inet.h>

#include <stdlib.h>
#include <list>

#include <pplib/audio/id3tag.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/mp3.h>
#include <pplib/audio/sample_formats.h>
#include <pplib/audio/decoder.h>

namespace pplib
{

PPLIBEXCEPTION(EncoderException, Exception);
PPLIBEXCEPTION(EncoderAlreadyStartedException, EncoderException);
PPLIBEXCEPTION(EncoderNotStartedException, EncoderException);
PPLIBEXCEPTION(EncoderBufferTooSmallException, EncoderException);
PPLIBEXCEPTION(EncoderPsychoAcousticException, EncoderException);
PPLIBEXCEPTION(EncoderInitializationException, EncoderException);
PPLIBEXCEPTION(EncoderAbortedException, EncoderException);
PPLIBEXCEPTION(EncoderAudioFormatMismatchException, EncoderException);

class Icecast
{
private:
    void* shout;
    bool bconnected;

public:
    Icecast();
    ~Icecast();
    String getVersion(int* major = NULL, int* minor = NULL, int* patch = NULL) const;
    String version() const;
    bool connected();
    void setConnection(const String& host, int port, const String& password);
    String host() const;
    int port() const;
    String password() const;
    void setMount(const String& mount);
    String mount() const;
    void setName(const String& name);
    String name() const;
    void setUrl(const String& url);
    String url() const;
    void setGenre(const String& genre);
    String genre() const;
    void setUser(const String& user);
    String user() const;
    void setAgent(const String& agent);
    String agent() const;
    void setDescription(const String& description);
    String description() const;
    void setDumpfile(const String& file);
    String dumpfile() const;
    void setAudioInfo(const String& name, const String& value);
    String audioInfo(const String& name) const;
    void setPublic(bool makepublic);
    void setFormatMP3();
    void setFormatOGG();
    void setNonBlocking(bool flag);

    void connect();
    void disconnect();

    void send(const void* buffer, size_t bytes);
    int delay();
    void sync();

    void sendMetadata(const String& name, const String& value);
    void setTitle(const String& title);
};

class AudioCD
{
private:
    void* cdio;
    String myDevice;
    size_t first_track_num;
    size_t i_tracks;
    size_t num_audio_tracks;
    size_t audio_frames;
    size_t last_lsn;
    void countAudioTracks();

public:
    PPLIBEXCEPTION(DeviceOpenFailed, Exception);
    PPLIBEXCEPTION(DeviceNotOpen, Exception);
    PPLIBEXCEPTION(InvalidAudioTrack, Exception);

    class Toc
    {
    public:
        uint8_t min, sec, frames;
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

    void openDevice(const String& device = String());
    void closeDevice();
    const String& deviceName() const;

    size_t firstTrack() const;
    size_t lastTrack() const;
    size_t numTotalTracks() const;
    size_t numAudioTracks() const;
    size_t totalAudioFrames() const;
    size_t totalAudioLength() const;
    size_t lastLsn() const;

    AudioCD::Track getTrack(int track);
    bool isAudioTrack(int track);

    static bool isSupported();
    static std::list<String> getDevices();
    static Toc lsn2toc(size_t lsn);
};

class CDDB
{
public:
    PPLIBEXCEPTION(QueryFailed, Exception);
    PPLIBEXCEPTION(InvalidDiscId, Exception);
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

        unsigned int discId;
        String category;
        String genre;
        int length;
        int year;
        String Artist;
        String Title;
        String Extra;
        TrackList Tracks;
    };

    typedef std::list<Disc> Matches;

private:
    pplib::Curl curl;
    String QueryPath, Server;
    String ClientName, ClientVersion;
    String UserName, Hostname;
    int port;
    String buildUri(const String& cmd);

public:
    CDDB();
    ~CDDB();

    void setHttpServer(const String& server, int port = 80);
    void setQueryPath(const String& path);
    void setProxy(const String& hostname, int port);
    void setClient(const String& name, const String& version);
    void setUser(const String& username, const String& hostname);

    int query(AudioCD& cd, Matches& list);
    void getDisc(unsigned int discId, const String& category, Disc& d);

    static bool isSupported();
    static unsigned int calcDiscId(AudioCD& cd);
};

class AudioEncoder
{
};

//!\brief Struktur zum Speichern des Fortschritts bei einem MP3-Encode-Vorgang
typedef struct tagSOUNDPROGRESS
{
    double timestarted;
    double timeend;
    double now;
    double past;
    float percent;
    float faktor;
    double eta;
    uint64_t position;
    uint64_t bytes;
    uint64_t position_thisfile;
    uint64_t bytes_thisfile;
    // PPL_WAVEHEADER *wav;
    // PPL_MPEG_HEADER *mpg;
} PPL_SOUNDPROGRESS;

class AudioEncoder_MP3
{
private:
    class FirstAudio
    {
    public:
        int frequency;
        int channels;
    };
    void* gfp;
    FirstAudio firstAudio;
    // PPL_SOUNDPROGRESS progress;
    FileObject* out;
    char* readcache;
    int mp3bufsize;
    unsigned char* mp3buf;

    uint32_t samples;

    void (*ProgressFunc)(int progress, void* priv);
    void* ProgressFuncPrivData;

    bool have_firstaudio;
    bool started;
    bool bStopEncode;

    void writeEncodedBytes(const char* buffer, size_t bytes);

public:
    AudioEncoder_MP3();
    ~AudioEncoder_MP3();
    void setVBR(int min = 32, int max = 320, int quality = 2);
    void setCBR(int kbps = 320, int quality = 2);
    void setABR(int kbps = 192, int quality = 2);
    void setQuality(int quality = 2);
    void setStereoMode(const AudioInfo::ChannelMode mode = AudioInfo::JOINT_STEREO);
    void setLowpassFreq(int freq = -1);  // -1=Disabled
    void setHighpassFreq(int freq = -1); // -1=Disabled
    void setProgressFunction(void (*ProgressFunc)(int progress, void* priv), void* priv);
    void startEncode(FileObject& output);
    void writeID3v2Tag(const ID3Tag& tag);
    void writeID3v1Tag(const ID3Tag& tag);
    void encode(AudioDecoder& decoder);
    void finish();
    void stop();

    void startEncode(int frequency, int channels);
    int encodeBuffer(SAMPLE16* left, SAMPLE16* right, int num, void* mp3buf, size_t buffersize);
    int encodeBuffer(STEREOSAMPLE16* buffer, int num, void* mp3buf, size_t buffersize);
    int flushBuffer(void* mp3buf, size_t buffersize);

    static const char* getLameVersion();
    static const char* getPSYVersion();
};

} // end of namespace pplib

#endif // _PPLIB_INCLUDE_SOUND
