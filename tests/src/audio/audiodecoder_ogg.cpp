/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLib).
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

#include <gtest/gtest.h>

#include <cmath>
#include <cstring>
#include <string>
#include <vector>

#include <config_pplib.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/decoder.h>
#include <pplib/audio/sample_formats.h>
#include <pplib/exceptions.h>
#include <pplib/core/file.h>
#include <pplib/core/memfile.h>

#ifdef HAVE_LIBVORBIS

namespace
{

//!\brief Anzahl Frames pro Ladeschritt bei den Ganzdatei-Vergleichen
constexpr size_t kChunk = 1009;

//!\brief normierter Float-Wert ([-1,1]) entspricht der 16 Bit Vollskala
constexpr float kFullScale = 32768.0f;

//!\brief Frames der ersten Kette von test_44kHz.ogg (Referenz: libvorbisfile)
constexpr size_t kStereoFrames = 173695;

/** @brief Dekodiert den Rest der Datei in einen Vektor
 *
 *  Der Schritt von 1009 Frames ist bewusst kein Vielfaches typischer Vorbis-
 *  Blockgroessen, damit Pufferuebergaenge quer ueber Blockgrenzen gecheckt werden.
 */
static void DecodeAll16(pplib::AudioDecoder& dec, std::vector<pplib::STEREOSAMPLE16>& out)
{
    out.clear();
    pplib::STEREOSAMPLE16 buf[kChunk];
    for (;;) {
        const size_t got = dec.getSamples(kChunk, buf);
        if (got == 0) break;
        out.insert(out.end(), buf, buf + got);
    }
}

static void DecodeAllFloat(pplib::AudioDecoder& dec, std::vector<pplib::STEREOSAMPLE_FLOAT>& out)
{
    out.clear();
    pplib::STEREOSAMPLE_FLOAT buf[kChunk];
    for (;;) {
        const size_t got = dec.getSamples(kChunk, buf);
        if (got == 0) break;
        out.insert(out.end(), buf, buf + got);
    }
}

/** @brief Liest eine komplette Testdatei in einen Vektor
 */
static std::vector<char> ReadWholeFile(const char* name)
{
    pplib::File file;
    const std::string path = std::string("testdata/audio/") + name;
    EXPECT_NO_THROW(file.open(path.c_str())) << "open() warf fuer " << path;
    const size_t size = (size_t)file.size();
    std::vector<char> data(size);
    EXPECT_EQ(size, file.read(data.data(), size));
    file.close();
    return data;
}

/** @brief Prueft, dass jedes Frame beide Kanalwerte gleich hat (Mono-Hoeherkopie)
 */
static bool LeftEqualsRight(const std::vector<pplib::STEREOSAMPLE16>& frames)
{
    for (size_t i = 0; i < frames.size(); i++)
        if (frames[i].left != frames[i].right) return false;
    return true;
}

static bool LeftEqualsRight(const std::vector<pplib::STEREOSAMPLE_FLOAT>& frames)
{
    for (size_t i = 0; i < frames.size(); i++)
        if (frames[i].left != frames[i].right) return false;
    return true;
}

/** @brief Testdatei, deren Lese- und Seek-Zugriffe auf Kommando eine I/O-Exception werfen
 *
 *  Die Callbacks des Decoders duerfen keine Exception durch die C-Frames von libvorbis
 *  hindurchreichen. Ein nicht abgefangener Throw waere undefiniertes Verhalten und
 *  braeche den Prozess ab.
 */
class FailingIoFile : public pplib::File
{
public:
    bool failRead{false};
    bool failSeek{false};
    bool failTell{false};

    size_t fread(void* ptr, size_t size, size_t nmemb) override
    {
        if (failRead) throw pplib::EndOfFileException();
        return pplib::File::fread(ptr, size, nmemb);
    }
    uint64_t seek(int64_t offset, pplib::FileObject::SeekOrigin origin) override
    {
        if (failSeek) throw pplib::EndOfFileException();
        return pplib::File::seek(offset, origin);
    }
    uint64_t tell() override
    {
        if (failTell) throw pplib::EndOfFileException();
        return pplib::File::tell();
    }
};

} // namespace

//!\brief Basis-Fixture fuer AudioDecoder_Ogg
class AudioDecoderOggTest : public ::testing::Test
{
protected:
    /** @brief Oeffnet eine echte Testdatei unter tests/testdata/audio */
    static void OpenTestFile(pplib::File& file, const char* name)
    {
        const std::string path = std::string("testdata/audio/") + name;
        ASSERT_NO_THROW(file.open(path.c_str())) << "open() warf fuer " << path;
        ASSERT_TRUE(file.isOpen()) << "Datei ist nicht geoeffnet: " << path;
    }

    /** @brief Oeffnet den Ogg-Dekoder fuer eine Testdatei */
    void Open(pplib::File& file, pplib::AudioDecoder_Ogg& dec, const char* name)
    {
        OpenTestFile(file, name);
        ASSERT_NO_THROW(dec.open(file));
    }
};

TEST_F(AudioDecoderOggTest, OpenFillsAudioInfoAndFactorySelectsOgg)
{
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz.ogg");

    pplib::AudioInfo info = dec.getAudioInfo();
    EXPECT_EQ(pplib::AudioInfo::AudioFormat::OGG, info.Format);
    EXPECT_EQ((uint64_t)2, info.Channels);
    EXPECT_EQ((uint64_t)16, info.BitsPerSample);
    EXPECT_EQ((uint64_t)4, info.BytesPerSample);
    EXPECT_EQ((uint64_t)4, info.BytesPerSample);
    EXPECT_EQ((uint64_t)44100, info.Frequency);
    EXPECT_EQ(pplib::AudioInfo::ChannelMode::STEREO, info.Mode);
    EXPECT_EQ((uint64_t)kStereoFrames, info.Samples);
    EXPECT_EQ((uint64_t)3938, info.Length); // 173695 Frames * 1000 / 44100 Hz
    EXPECT_EQ(pplib::AudioInfo::SampleEncoding::INTEGER, info.SampleType);
    EXPECT_EQ(pplib::AudioInfo::ChannelMode::STEREO, info.Mode);

    pplib::File file2;
    OpenTestFile(file2, "test_44kHz.ogg");
    pplib::AudioDecoder* made = pplib::GetAudioDecoder(file2);
    ASSERT_NE(nullptr, made);
    EXPECT_NE(nullptr, dynamic_cast<pplib::AudioDecoder_Ogg*>(made));
    EXPECT_EQ((uint64_t)kStereoFrames, made->getAudioInfo().Samples);
    delete made;
}

TEST_F(AudioDecoderOggTest, ReopenAfterCloseDecodesTheSameFrames)
{
    // open() nach close() muss denselben Anfang liefern; bisher stuerzte der Aufruf,
    // weil close() den Handle auf NULL setzt und open() ihn bedingungslos verwendet.
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz.ogg");

    pplib::STEREOSAMPLE16 first[8] = {};
    pplib::STEREOSAMPLE16 again[8] = {};
    ASSERT_EQ((size_t)8, dec.getSamples(8, first));
    dec.close();
    EXPECT_EQ((size_t)0, dec.getPosition());

    ASSERT_NO_THROW(dec.open(file));
    ASSERT_EQ((size_t)8, dec.getSamples(8, again));
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(first[i].left, again[i].left) << "Frame " << i;
        EXPECT_EQ(first[i].right, again[i].right) << "Frame " << i;
    }
    EXPECT_EQ((size_t)8, dec.getPosition());
}

TEST_F(AudioDecoderOggTest, EveryOpenRoundReleasesThePreviousHandle)
{
    // Ein zweites open() ohne close() gab den libvorbisfile-Handle nie frei
    // (rund 69 kB pro Runde). zusaetzlich lehnt open() eine 5.1-Kanalzahl ab und
    // muss den dabei geoeffneten Handle zuraeumgeraeumt haben.
    pplib::File file;
    OpenTestFile(file, "test_44kHz.ogg");
    pplib::AudioInfo fake;
    ASSERT_NO_THROW(pplib::IdentAudioFile(file, fake));
    fake.Channels = 0;

    pplib::AudioDecoder_Ogg dec;
    pplib::STEREOSAMPLE16 buf[8];
    for (int round = 0; round < 3; round++) {
        ASSERT_NO_THROW(dec.open(file));
        ASSERT_EQ((size_t)8, dec.getSamples(8, buf));
        // ohne vorheriges close(): das naechste open() uebernimmt die Freigabe
        EXPECT_THROW(dec.open(file, &fake), pplib::UnsupportedAudioFormatException);
        ASSERT_NO_THROW(dec.open(file));
        ASSERT_EQ((size_t)8, dec.getSamples(8, buf));
    }
}

TEST_F(AudioDecoderOggTest, AllReadPathsThrowAfterClose)
{
    // Nach close() ist kein Handle mehr da: erwartet wird DecoderException statt
    // NULL-Zugriff, analog zu AudioDecoder_Flac.
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz.ogg");
    pplib::STEREOSAMPLE16 s16[8];
    pplib::STEREOSAMPLE32 s32[8] = {};
    pplib::STEREOSAMPLE_FLOAT sflt[8] = {};
    ASSERT_EQ((size_t)8, dec.getSamples(8, s16));
    dec.close();

    EXPECT_THROW(dec.getSamples(8, s16), pplib::DecoderException);
    EXPECT_THROW(dec.addSamples(8, s32), pplib::DecoderException);
    EXPECT_THROW(dec.getSamples(8, sflt), pplib::DecoderException);
    EXPECT_THROW(dec.addSamples(8, sflt), pplib::DecoderException);
}

TEST_F(AudioDecoderOggTest, SeekAfterCloseThrowsDecoderException)
{
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz.ogg");
    dec.close();
    EXPECT_THROW(dec.seekSample(1000), pplib::DecoderException);
}

TEST_F(AudioDecoderOggTest, AllReadPathsThrowBeforeOpen)
{
    pplib::AudioDecoder_Ogg dec;
    pplib::STEREOSAMPLE16 s16[8];
    pplib::STEREOSAMPLE32 s32[8] = {};
    pplib::STEREOSAMPLE_FLOAT sflt[8] = {};

    EXPECT_THROW(dec.getSamples(8, s16), pplib::DecoderException);
    EXPECT_THROW(dec.addSamples(8, s32), pplib::DecoderException);
    EXPECT_THROW(dec.getSamples(8, sflt), pplib::DecoderException);
    EXPECT_THROW(dec.addSamples(8, sflt), pplib::DecoderException);
    EXPECT_THROW(dec.seekSample(8), pplib::DecoderException);
    EXPECT_EQ((size_t)0, dec.getPosition());
}

TEST_F(AudioDecoderOggTest, MonoFileUpmixesBothChannels)
{
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz_mono.ogg");

    EXPECT_EQ((uint64_t)1, dec.getAudioInfo().Channels);
    EXPECT_EQ((uint64_t)2, dec.getAudioInfo().BytesPerSample);
    EXPECT_EQ(pplib::AudioInfo::ChannelMode::MONO, dec.getAudioInfo().Mode);
    EXPECT_EQ(pplib::AudioInfo::SampleEncoding::INTEGER, dec.getAudioInfo().SampleType);

    std::vector<pplib::STEREOSAMPLE16> got;
    DecodeAll16(dec, got);
    ASSERT_GT(got.size(), (size_t)100000);
    EXPECT_EQ(dec.getAudioInfo().Samples, (uint64_t)got.size());
    EXPECT_TRUE(LeftEqualsRight(got));

    // Der Float-Pfad hat eine eigene Mono-Verzweigung und mischt ebenso auf zwei Kanaele hoch
    ASSERT_NO_THROW(dec.seekSample(0));
    std::vector<pplib::STEREOSAMPLE_FLOAT> f;
    DecodeAllFloat(dec, f);
    EXPECT_EQ(got.size(), f.size());
    EXPECT_TRUE(LeftEqualsRight(f));
}

TEST_F(AudioDecoderOggTest, MemFileDecodesLikeARealFile)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz.ogg");
    std::vector<char> data = ReadWholeFile("test_44kHz.ogg");
    ASSERT_FALSE(data.empty());
    pplib::MemFile mem(data.data(), data.size());
    ASSERT_TRUE(mem.isOpen());

    std::vector<pplib::STEREOSAMPLE16> ref;
    {
        pplib::AudioDecoder_Ogg dec;
        ASSERT_NO_THROW(dec.open(file));
        DecodeAll16(dec, ref);
    }
    ASSERT_EQ((size_t)kStereoFrames, ref.size());

    std::vector<pplib::STEREOSAMPLE16> got;
    {
        pplib::AudioDecoder_Ogg dec;
        ASSERT_NO_THROW(dec.open(mem));
        DecodeAll16(dec, got);
    }
    ASSERT_EQ(ref.size(), got.size());
    size_t diff = 0;
    for (size_t i = 0; i < ref.size(); i++)
        if (memcmp(&ref[i], &got[i], sizeof(pplib::STEREOSAMPLE16)) != 0) diff++;
    EXPECT_EQ((size_t)0, diff);
}

/** @brief Die Float-Pfade liefern dieselben Werte wie die 16-Bit-Pfade, nur normiert
 *
 *  libvorbisfile rundet die Decoderwerte fuer ov_read() auf 16 Bit; die Float-Pfade
 *  liefern dieselben Werte geteilt durch die 16-Bit-Vollskala.
 */
TEST_F(AudioDecoderOggTest, FloatPathScalesLikeSixteenBitPath)
{
    constexpr size_t kFrames = 2000;
    pplib::File file;
    OpenTestFile(file, "test_44kHz.ogg");

    std::vector<pplib::STEREOSAMPLE16> i16(kFrames);
    {
        pplib::AudioDecoder_Ogg dec;
        ASSERT_NO_THROW(dec.open(file));
        ASSERT_EQ(kFrames, dec.getSamples(kFrames, i16.data()));
    }

    std::vector<pplib::STEREOSAMPLE_FLOAT> flt(kFrames);
    {
        pplib::AudioDecoder_Ogg dec;
        ASSERT_NO_THROW(dec.open(file));
        ASSERT_EQ(kFrames, dec.getSamples(kFrames, flt.data()));
        dec.seekSample(0);
        std::vector<pplib::STEREOSAMPLE_FLOAT> zero(kFrames, {0.0f, 0.0f});
        ASSERT_EQ(kFrames, dec.addSamples(kFrames, zero.data()));
        for (size_t i = 0; i < kFrames; i++) {
            EXPECT_EQ(zero[i].left, flt[i].left) << "Frame " << i;
            EXPECT_EQ(zero[i].right, flt[i].right) << "Frame " << i;
        }
    }

    for (size_t i = 0; i < kFrames; i++) {
        EXPECT_NEAR((float)i16[i].left / kFullScale, flt[i].left, 1.0f / kFullScale) << "links, Frame " << i;
        EXPECT_NEAR((float)i16[i].right / kFullScale, flt[i].right, 1.0f / kFullScale) << "rechts, Frame " << i;
        EXPECT_LE(std::abs(flt[i].left), 1.0f) << "Frame " << i;
    }
}

/** @brief Zahlt position und Rueckgabewerte beider Lesepfade auf die Framezahl ein */
TEST_F(AudioDecoderOggTest, PositionTracksFramesInBothReadPaths)
{
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz.ogg");

    pplib::STEREOSAMPLE16 buf16[kChunk];
    size_t frames = 0;
    for (int step = 0; step < 3; step++) {
        const size_t got = dec.getSamples(kChunk, buf16);
        ASSERT_GT(got, (size_t)0) << "Schritt " << step;
        frames += got;
        EXPECT_EQ(frames, dec.getPosition()) << "16-Bit-Pfad, Schritt " << step;
    }

    std::vector<pplib::STEREOSAMPLE16> rest;
    DecodeAll16(dec, rest);
    EXPECT_EQ(kStereoFrames, frames + rest.size());
    EXPECT_EQ((uint64_t)kStereoFrames, dec.getPosition());

    dec.close();
    ASSERT_NO_THROW(dec.open(file));
    std::vector<pplib::STEREOSAMPLE_FLOAT> flt;
    DecodeAllFloat(dec, flt);
    EXPECT_EQ(kStereoFrames, flt.size());
    EXPECT_EQ((uint64_t)kStereoFrames, dec.getPosition());
}

/** @brief Eine Anfrage ueber das Dateiende hinaus liefert nur die vorhandenen Frames
 *
 *  Die fruehere bytemaessige Rest-Rechnung lief ueber int und ergab bei grosem @p num
 *  eine negative Framezahl. Der Puffer ist exakt so gross wie die zu fuellende Framezahl.
 */
TEST_F(AudioDecoderOggTest, HugeRequestReturnsOnlyAvailableFrames)
{
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz.ogg");
    const size_t huge = (size_t)1 << 40;

    std::vector<pplib::STEREOSAMPLE16> s16(kStereoFrames);
    EXPECT_EQ(kStereoFrames, dec.getSamples(huge, s16.data()));
    EXPECT_EQ((uint64_t)kStereoFrames, dec.getPosition());

    dec.close();
    ASSERT_NO_THROW(dec.open(file));
    std::vector<pplib::STEREOSAMPLE_FLOAT> sflt(kStereoFrames);
    EXPECT_EQ(kStereoFrames, dec.getSamples(huge, sflt.data()));
    EXPECT_EQ((uint64_t)kStereoFrames, dec.getPosition());

    // Beide Aufaddier-Pfade begrenzen genauso, auch wenn der Zielpuffer voll besetzt wird
    dec.close();
    ASSERT_NO_THROW(dec.open(file));
    std::vector<pplib::STEREOSAMPLE32> add32(kStereoFrames);
    EXPECT_EQ(kStereoFrames, dec.addSamples(huge, add32.data()));
    EXPECT_EQ((uint64_t)kStereoFrames, dec.getPosition());

    dec.close();
    ASSERT_NO_THROW(dec.open(file));
    std::vector<pplib::STEREOSAMPLE_FLOAT> addflt(kStereoFrames);
    EXPECT_EQ(kStereoFrames, dec.addSamples(huge, addflt.data()));
    EXPECT_EQ((uint64_t)kStereoFrames, dec.getPosition());

    pplib::STEREOSAMPLE16 once[4];
    EXPECT_EQ((size_t)0, dec.getSamples(4, once));
}

/** @brief Bei verketteten Dateien beschreiben Samples und Length nur die erste Kette */
TEST_F(AudioDecoderOggTest, ChainedFileReportsFirstChainInAudioInfo)
{
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz_chained.ogg");

    const pplib::AudioInfo info = dec.getAudioInfo();
    EXPECT_EQ((uint64_t)kStereoFrames, info.Samples);
    EXPECT_EQ((uint64_t)3938, info.Length);
    EXPECT_EQ((uint64_t)2, info.Channels); // Kanaele der ersten Kette
}

/** @brief Eine verkettete Datei wird bis zum Ende der ersten Kette abgespielt */
TEST_F(AudioDecoderOggTest, ChainedFileDecodesOnlyFirstChain)
{
    std::vector<pplib::STEREOSAMPLE16> plain;
    {
        pplib::File file;
        pplib::AudioDecoder_Ogg dec;
        Open(file, dec, "test_44kHz.ogg");
        DecodeAll16(dec, plain);
    }
    ASSERT_EQ((size_t)kStereoFrames, plain.size());

    std::vector<pplib::STEREOSAMPLE16> chained;
    {
        pplib::File file;
        pplib::AudioDecoder_Ogg dec;
        Open(file, dec, "test_44kHz_chained.ogg");
        DecodeAll16(dec, chained);
    }
    ASSERT_EQ(plain.size(), chained.size());
    size_t diff = 0;
    for (size_t i = 0; i < plain.size(); i++)
        if (memcmp(&plain[i], &chained[i], sizeof(pplib::STEREOSAMPLE16)) != 0) diff++;
    EXPECT_EQ((size_t)0, diff) << "erste Kette unterscheidet sich von der Einzeldatei";
}

/** @brief Der Float-Pfad endet an der ersten Kette und lese nicht die Kanale der zweiten */
TEST_F(AudioDecoderOggTest, ChainedFloatPathStopsAtFirstChain)
{
    std::vector<pplib::STEREOSAMPLE_FLOAT> plain;
    {
        pplib::File file;
        pplib::AudioDecoder_Ogg dec;
        Open(file, dec, "test_44kHz.ogg");
        DecodeAllFloat(dec, plain);
    }
    ASSERT_EQ((size_t)kStereoFrames, plain.size());

    std::vector<pplib::STEREOSAMPLE_FLOAT> chained;
    {
        pplib::File file;
        pplib::AudioDecoder_Ogg dec;
        Open(file, dec, "test_44kHz_chained.ogg");
        DecodeAllFloat(dec, chained);
    }
    ASSERT_EQ(plain.size(), chained.size());
    size_t diff = 0;
    for (size_t i = 0; i < plain.size(); i++) {
        if (plain[i].left != chained[i].left || plain[i].right != chained[i].right) diff++;
    }
    EXPECT_EQ((size_t)0, diff) << "erste Kette unterscheidet sich von der Einzeldatei";
}

/** @brief seekSample() lehnt Ziele hinter dem Ende der ersten Kette ab */
TEST_F(AudioDecoderOggTest, SeekBeyondFirstChainThrows)
{
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz.ogg");

    // ov_pcm_seek() setzte solche Ziele stillgueltig um; der Dekoder spielte dann aus
    // einer spaeteren Kette weiter
    EXPECT_THROW(dec.seekSample(kStereoFrames + 100000), pplib::DecoderException);
    EXPECT_THROW(dec.seekSample(2000000), pplib::DecoderException);
    EXPECT_EQ((uint64_t)0, dec.getPosition());

    ASSERT_NO_THROW(dec.seekSample(kStereoFrames - 1));
    EXPECT_EQ((uint64_t)(kStereoFrames - 1), dec.getPosition());
    pplib::STEREOSAMPLE16 tail[4];
    EXPECT_EQ((size_t)1, dec.getSamples(4, tail));
}

/** @brief Ziel am Ende der ersten Kette gilt als Dateiende und liefert 0 Frames */
TEST_F(AudioDecoderOggTest, SeekToChainEndReadsNoFrames)
{
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz_chained.ogg");

    ASSERT_NO_THROW(dec.seekSample(kStereoFrames));
    pplib::STEREOSAMPLE16 s16[4];
    pplib::STEREOSAMPLE_FLOAT sflt[4] = {};
    EXPECT_EQ((size_t)0, dec.getSamples(4, s16));
    EXPECT_EQ((size_t)0, dec.getSamples(4, sflt));
    EXPECT_EQ((size_t)0, dec.addSamples(4, sflt));
    EXPECT_EQ((uint64_t)kStereoFrames, dec.getPosition());
}

/** @brief I/O-Fehler beim Lesen beendet den Strom wie Dateiende, ohne zu werfen
 *
 *  Die Callbacks melden Kurzlesen als Ende; der Dekoder liefert die noch dekodierten
 *  Frames und danach 0. Eine I/O-Exception darf die Lesepfade nicht verlassen.
 */
TEST_F(AudioDecoderOggTest, IoFailureDuringReadEndsStreamLikeEof)
{
    FailingIoFile file;
    pplib::AudioDecoder_Ogg dec;
    OpenTestFile(file, "test_44kHz.ogg");
    ASSERT_NO_THROW(dec.open(file));

    pplib::STEREOSAMPLE16 buf[512];
    size_t before = 0;
    ASSERT_EQ((size_t)512, dec.getSamples(512, buf));
    const uint64_t posBefore = dec.getPosition();
    ASSERT_EQ((uint64_t)512, posBefore);

    file.failRead = true;
    size_t got = 512;
    while (got > 0)
        EXPECT_NO_THROW(got = dec.getSamples(512, buf));
    EXPECT_EQ((size_t)0, dec.getSamples(512, buf));
    EXPECT_LT(dec.getPosition(), (uint64_t)kStereoFrames);
    EXPECT_GE(dec.getPosition(), posBefore);
    EXPECT_NO_THROW(dec.close());
}

/** @brief I/O-Fehler beim Seek fuehrt zu DecoderException, nicht zur I/O-Exception
 */
TEST_F(AudioDecoderOggTest, IoFailureDuringSeekThrowsDecoderException)
{
    FailingIoFile file;
    pplib::AudioDecoder_Ogg dec;
    OpenTestFile(file, "test_44kHz.ogg");
    ASSERT_NO_THROW(dec.open(file));

    pplib::STEREOSAMPLE16 buf[16];
    ASSERT_EQ((size_t)16, dec.getSamples(16, buf));

    file.failSeek = true;
    EXPECT_THROW(dec.seekSample(20000), pplib::DecoderException);
    EXPECT_EQ((uint64_t)16, dec.getPosition());

    // Lesefehler waehrend des Seeks: libvorbis schlaeft, der Dekoder meldet DecoderException
    file.failSeek = false;
    file.failRead = true;
    try {
        dec.seekSample(30000);
    }
    catch (const pplib::DecoderException&) {
    }
    EXPECT_NO_THROW(dec.close());
}

/** @brief Mehr als zwei Kanaele: die Erkennung meldet sie, der Dekoder lehnt ab
 *
 *  Entscheidung: Erkennung und Unterstuetzung sind getrennt. IdentAudioFile() gibt die
 *  Kanalzahl des Bitstroms zurueck, abgelehnt wird erst in open() - danach darf kein
 *  halboeffener Handle zurueckbleiben, auch nicht ueber die Factory.
 */
TEST_F(AudioDecoderOggTest, MultichannelFileIsIdentifiedButNotOpened)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz_51.ogg");

    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ((uint64_t)6, info.Channels);
    EXPECT_GT(info.Samples, (uint64_t)0);

    pplib::AudioDecoder_Ogg dec;
    EXPECT_THROW(dec.open(file), pplib::UnsupportedAudioFormatException);
    EXPECT_THROW(dec.seekSample(0), pplib::DecoderException);
    pplib::STEREOSAMPLE16 buf[8];
    EXPECT_THROW(dec.getSamples(8, buf), pplib::DecoderException);

    // Die Factory faengt die Ablehnung ab und liefert kein halbfertiges Objekt
    pplib::AudioDecoder* auto_dec = pplib::GetAudioDecoder(file);
    EXPECT_EQ(nullptr, auto_dec);
    delete auto_dec;
}

/** @brief Eine angeschnittene Datei wird erkannt, meldet aber keine Samplezahl */
TEST_F(AudioDecoderOggTest, TruncatedFileReportsNoSamplesAndReadsNothing)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz_truncated.ogg");

    pplib::AudioInfo info;
    const bool identified = pplib::IdentAudioFile(file, info);
    if (!identified) {
        // Kurze Rumpfstuecke ohne vollstaendigen Header werden gar nicht erst erkannt;
        // dann ist nichts weiter zu pruefen.
        EXPECT_EQ(pplib::AudioInfo::AudioFormat::UNKNOWN, info.Format);
        return;
    }
    EXPECT_EQ((uint64_t)0, info.Samples);
    EXPECT_EQ((uint32_t)0, info.Length);

    pplib::AudioDecoder_Ogg dec;
    ASSERT_NO_THROW(dec.open(file));
    EXPECT_EQ((uint64_t)0, dec.getAudioInfo().Samples);
    pplib::STEREOSAMPLE16 buf[16];
    EXPECT_EQ((size_t)0, dec.getSamples(16, buf));
    EXPECT_NO_THROW(dec.close());
}

/** @brief Bitrate-Management: untere und obere Bitrate gleich, ISVBR bleibt false.
 *
 * Die mit <tt>ffmpeg -b:a 192k</tt> kodierte Datei fuehrt den Zweig fuer gleiche untere und
 * obere Bitrate aus: sie gilt als konstante Bitrate, ISVBR bleibt false und Bitrate steht
 * auf 192 kbit/s statt auf der nominalen Bitrate.
 */
TEST_F(AudioDecoderOggTest, ConstantBitrateFileIsNotReportedAsVBR)
{
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz_cbr.ogg");

    pplib::AudioInfo info = dec.getAudioInfo();
    EXPECT_FALSE(info.IsVBR);
    EXPECT_EQ((uint16_t)192, info.Bitrate);
    EXPECT_EQ((uint64_t)173695, info.Samples);
    EXPECT_EQ((uint32_t)3938, info.Length);

    std::vector<pplib::STEREOSAMPLE16> got;
    DecodeAll16(dec, got);
    EXPECT_EQ(info.Samples, (uint64_t)got.size());
}

/** @brief Die Fabrik lehnt ab, ohne ein halbfertiges Objekt zurueckzugeben.
 *
 * Bei einer Datei mit sechs Kanaeln erkennt `IdentAudioFile()` das Format, `open()` lehnt
 * sie ab. Die Fabrik muss in diesem Fall NULL liefern; zusaetzlich darf der abgelehnte
 * Decoder keinen libvorbisfile-Zustand zuruckbehalten (LeakSanitizer im ASan-Build).
 */
TEST_F(AudioDecoderOggTest, FactoryReturnsNothingForAFileTheDecoderRejects)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz_51.ogg");

    pplib::AudioDecoder* dec = pplib::GetAudioDecoder(file);
    EXPECT_EQ((pplib::AudioDecoder*)NULL, dec);
    delete dec;
}

/** @brief Ein Kanal wird auch beim Aufaddieren in beiden Ausgaeben angeboten */
TEST_F(AudioDecoderOggTest, AddSamplesMonoFileFeedsBothChannels)
{
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz_mono.ogg");

    std::vector<pplib::STEREOSAMPLE32> dst(200);
    for (size_t i = 0; i < dst.size(); i++) {
        dst[i].left = 1;
        dst[i].right = 1;
    }
    ASSERT_EQ(dst.size(), dec.addSamples(dst.size(), dst.data()));
    bool same = true;
    for (size_t i = 0; i < dst.size(); i++)
        if (dst[i].left != dst[i].right) same = false;
    EXPECT_TRUE(same);
    EXPECT_LT((int32_t)1, dst[4].left);
}

/** @brief Null-Puffer werden in allen vier Lesepfaden abgewiesen, auch im offenen Zustand */
TEST_F(AudioDecoderOggTest, NullBufferIsRejectedByEveryReadPath)
{
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz.ogg");

    EXPECT_THROW(dec.getSamples(8, (pplib::STEREOSAMPLE16*)NULL), pplib::DecoderException);
    EXPECT_THROW(dec.addSamples(8, (pplib::STEREOSAMPLE32*)NULL), pplib::DecoderException);
    EXPECT_THROW(dec.getSamples(8, (pplib::STEREOSAMPLE_FLOAT*)NULL), pplib::DecoderException);
    EXPECT_THROW(dec.addSamples(8, (pplib::STEREOSAMPLE_FLOAT*)NULL), pplib::DecoderException);
    EXPECT_NO_THROW(dec.close());
}

/** @brief Der STEREOSAMPLE32-Pfad addiert auf vorhandene Werte und zaehlt Frames */
TEST_F(AudioDecoderOggTest, AddSamplesAddsIntoStereosample32Buffer)
{
    constexpr size_t n = 4000;
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz.ogg");

    std::vector<pplib::STEREOSAMPLE16> ref(n);
    ASSERT_EQ(n, dec.getSamples(n, ref.data()));
    ASSERT_NO_THROW(dec.seekSample(0));

    std::vector<pplib::STEREOSAMPLE32> acc(n);
    for (size_t i = 0; i < n; i++) {
        acc[i].left = 7;
        acc[i].right = 11;
    }
    EXPECT_EQ(n, dec.addSamples(n, acc.data()));
    for (size_t i = 0; i < n; i++) {
        EXPECT_EQ((int32_t)ref[i].left + 7, acc[i].left) << "Frame " << i;
        EXPECT_EQ((int32_t)ref[i].right + 11, acc[i].right) << "Frame " << i;
    }
    EXPECT_EQ((uint64_t)n, dec.getPosition());
    EXPECT_NO_THROW(dec.close());
}

/** @brief Der Float-Addierpfad mischt Mono auf beide Kanaele hoch und addiert */
TEST_F(AudioDecoderOggTest, AddSamplesFloatMonoUpsamplesAndAdds)
{
    constexpr size_t n = 2000;
    pplib::File file;
    pplib::AudioDecoder_Ogg dec;
    Open(file, dec, "test_44kHz_mono.ogg");

    std::vector<pplib::STEREOSAMPLE_FLOAT> ref(n);
    ASSERT_EQ(n, dec.getSamples(n, ref.data()));
    ASSERT_NO_THROW(dec.seekSample(0));

    std::vector<pplib::STEREOSAMPLE_FLOAT> acc(n);
    for (size_t i = 0; i < n; i++) {
        acc[i].left = 0.5f;
        acc[i].right = -0.25f;
    }
    EXPECT_EQ(n, dec.addSamples(n, acc.data()));
    for (size_t i = 0; i < n; i++) {
        EXPECT_NEAR(ref[i].left + 0.5f, acc[i].left, 1e-4) << "Frame " << i;
        EXPECT_NEAR(ref[i].left - 0.25f, acc[i].right, 1e-4) << "Frame " << i;
    }
    EXPECT_NO_THROW(dec.close());
}

/** @brief Eine Datei ohne Ogg-Vorbis-Header wird abgelehnt und hinterlasst keinen Handle */
TEST_F(AudioDecoderOggTest, OpenRejectsFileWithoutOggVorbisStream)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz.wav");

    pplib::AudioDecoder_Ogg dec;
    EXPECT_THROW(dec.open(file), pplib::UnsupportedAudioFormatException);
    EXPECT_EQ((size_t)0, dec.getPosition());

    // Nach der Ablehnung besteht derselbe Zustand wie vor jedem open()
    pplib::STEREOSAMPLE16 buf[8];
    EXPECT_THROW(dec.getSamples(8, buf), pplib::DecoderException);
    EXPECT_NO_THROW(dec.close());
    EXPECT_NO_THROW(dec.close());

    // Das naechste open() auf einer echten Ogg-Datei funktioniert unveraendert
    pplib::File ogg;
    OpenTestFile(ogg, "test_44kHz.ogg");
    ASSERT_NO_THROW(dec.open(ogg));
    EXPECT_EQ((size_t)8, dec.getSamples(8, buf));
    dec.close();
}

/** @brief Wirft der Tell-Callback, gilt der Seek als Fehler statt als Exception durch C-Frames */
TEST_F(AudioDecoderOggTest, IoFailureInTellCallbackBecomesDecoderException)
{
    FailingIoFile file;
    pplib::AudioDecoder_Ogg dec;
    OpenTestFile(file, "test_44kHz.ogg");
    ASSERT_NO_THROW(dec.open(file));
    file.failTell = true;
    EXPECT_THROW(dec.seekSample(20000), pplib::DecoderException);
    EXPECT_NO_THROW(dec.close());
}

#endif // HAVE_LIBVORBIS
