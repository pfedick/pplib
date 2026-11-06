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

#include <gtest/gtest.h>

#include <cmath>
#include <cstring>
#include <string>
#include <algorithm>
#include <vector>

#include <config_pplib.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/decoder.h>
#include <pplib/audio/sample_formats.h>
#include <pplib/exceptions.h>
#include <pplib/core/file.h>
#include <pplib/core/memfile.h>

#ifdef HAVE_MPG123

namespace
{

//!\brief Anzahl Frames pro Ladeschritt bei den Ganzdatei-Vergleichen
constexpr size_t kChunk = 1009;

//!\brief normierter Float-Wert ([-1,1]) entspricht der 16 Bit Vollskala
constexpr float kFullScale = 32768.0f;

/** @brief Dekodiert den Rest der Datei in einen Vektor
 *
 *  Der Schritt von 1009 Frames ist bewusst kein Vielfaches der MP3-Rahmengroesse
 *  (576 oder 1152 Samples), damit Pufferuebergaenge quer ueber Rahmengrenzen
 *  geprüft werden.
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

static void Compare16(const std::vector<pplib::STEREOSAMPLE16>& ref, const std::vector<pplib::STEREOSAMPLE16>& got, size_t offset)
{
    ASSERT_EQ(ref.size(), got.size());
    for (size_t i = 0; i < ref.size(); i++) {
        EXPECT_EQ(ref[i].left, got[i].left) << "Frame " << (offset + i);
        EXPECT_EQ(ref[i].right, got[i].right) << "Frame " << (offset + i);
    }
}

} // namespace

//!\brief Basis-Fixture fuer AudioDecoder_MP3
class AudioDecoderMp3Test : public ::testing::Test
{
protected:
    /** @brief Oeffnet eine echte Testdatei unter tests/testdata/audio */
    static void OpenTestFile(pplib::File& file, const char* name)
    {
        const std::string path = std::string("testdata/audio/") + name;
        ASSERT_NO_THROW(file.open(path.c_str())) << "open() warf fuer " << path;
        ASSERT_TRUE(file.isOpen()) << "Datei ist nicht geoeffnet: " << path;
    }

    /** @brief Oeffnet den MP3-Dekoder fuer eine Testdatei */
    void Open(pplib::File& file, pplib::AudioDecoder_MP3& dec, const char* name)
    {
        OpenTestFile(file, name);
        ASSERT_NO_THROW(dec.open(file));
    }
};

TEST_F(AudioDecoderMp3Test, OpenFillsAudioInfoAndFactorySelectsMp3)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_192cbr.mp3");

    pplib::AudioInfo info = dec.getAudioInfo();
    EXPECT_EQ(pplib::AudioInfo::AudioFormat::MP3, info.Format);
    EXPECT_EQ((uint64_t)2, info.Channels);
    EXPECT_EQ((uint64_t)16, info.BitsPerSample);
    EXPECT_EQ((uint64_t)44100, info.Frequency);
    EXPECT_EQ((uint64_t)0, info.AudioStart);

    // Der Fabrik-Dekoder muss dasselbe Format liefern
    pplib::File file2;
    OpenTestFile(file2, "test_192cbr.mp3");
    pplib::AudioDecoder* made = pplib::GetAudioDecoder(file2);
    ASSERT_NE(nullptr, made);
    EXPECT_NE(nullptr, dynamic_cast<pplib::AudioDecoder_MP3*>(made));
    EXPECT_EQ((uint64_t)176223, made->getAudioInfo().Samples);
    delete made;
}

TEST_F(AudioDecoderMp3Test, TaggedFileSkipsId3HeaderAndMatchesUntaggedSamples)
{
    pplib::File plainFile;
    pplib::AudioDecoder_MP3 plain;
    Open(plainFile, plain, "test_192cbr.mp3");
    std::vector<pplib::STEREOSAMPLE16> ref;
    DecodeAll16(plain, ref);
    ASSERT_LT((size_t)170000, ref.size());

    pplib::File taggedFile;
    pplib::AudioDecoder_MP3 tagged;
    Open(taggedFile, tagged, "test_192cbr_tagged.mp3");
    EXPECT_EQ((uint64_t)1024, tagged.getAudioInfo().AudioStart);

    std::vector<pplib::STEREOSAMPLE16> got;
    DecodeAll16(tagged, got);
    Compare16(ref, got, 0);
}

TEST_F(AudioDecoderMp3Test, MonoFileKeepsSampleRateAndReplicatesChannel)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_22khz_mono.mp3");
    EXPECT_EQ((uint64_t)1, dec.getAudioInfo().Channels);
    EXPECT_EQ((uint64_t)22050, dec.getAudioInfo().Frequency);

    std::vector<pplib::STEREOSAMPLE16> got;
    DecodeAll16(dec, got);
    // 21935 Frames entsprechen einer Sekunde bei 22050 Hz. Wuerde libmpg123 auf
    // 44100 Hz hochresampeln, Haette man rund doppelt so viele Frames.
    ASSERT_LT((size_t)20000, got.size());
    ASSERT_GT((size_t)24000, got.size());
    EXPECT_EQ((uint64_t)1, dec.getAudioInfo().Channels);
    EXPECT_EQ((uint64_t)22050, dec.getAudioInfo().Frequency);
    for (size_t i = 0; i < got.size(); i++) {
        ASSERT_EQ(got[i].left, got[i].right) << "Frame " << i;
    }
}

TEST_F(AudioDecoderMp3Test, LowRateStereoFileIsNotResampledOrMixedDown)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_32khz_stereo.mp3");
    EXPECT_EQ((uint64_t)2, dec.getAudioInfo().Channels);
    EXPECT_EQ((uint64_t)32000, dec.getAudioInfo().Frequency);

    std::vector<pplib::STEREOSAMPLE16> got;
    DecodeAll16(dec, got);
    // Nach libmpg123-Suche endet der Stream am letzten vollen Rahmen; die
    // Xing-Angabe (34560 Frames) liegt darueber.
    ASSERT_LT((size_t)30000, got.size());
    ASSERT_GT((size_t)36000, got.size());

    // Weder Abtastrate noch Kanalzahl werden veraendert, es wird also weder
    // hoch- oder runtergetaktet noch auf einen Kanal heruntergemischt.
    EXPECT_EQ((uint64_t)2, dec.getAudioInfo().Channels);
    EXPECT_EQ((uint64_t)32000, dec.getAudioInfo().Frequency);
}

TEST_F(AudioDecoderMp3Test, StereoChannelsAreKeptApart)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_192cbr.mp3");

    std::vector<pplib::STEREOSAMPLE16> got;
    DecodeAll16(dec, got);
    ASSERT_LT((size_t)170000, got.size());

    // Ein Downmix auf einen Kanal und anschiessende Replikation würde links gleich rechts machen
    size_t different = 0;
    for (size_t i = 0; i < got.size(); i++) {
        if (got[i].left != got[i].right) different++;
    }
    EXPECT_LT((size_t)(got.size() / 4), different);
}

TEST_F(AudioDecoderMp3Test, AddSamplesMixesUnscaledOntoExistingValues)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_192cbr.mp3");

    // Referenz zuerst linear holen, dann zurckspulen und auf dieselben Frames aufaddieren
    pplib::STEREOSAMPLE16 ref[512];
    ASSERT_EQ((size_t)512, dec.getSamples(512, ref));
    dec.seekSample(0);

    pplib::STEREOSAMPLE32 mixed[512];
    for (size_t i = 0; i < 512; i++) {
        mixed[i].left = 5;
        mixed[i].right = -5;
    }
    ASSERT_EQ((size_t)512, dec.addSamples(512, mixed));

    // Samples des 16 Bit Decoders stehen bereits auf der 16 Bit Ebene des Mischpuffers
    for (size_t i = 0; i < 512; i++) {
        EXPECT_EQ((int32_t)ref[i].left + 5, mixed[i].left) << "Frame " << i;
        EXPECT_EQ((int32_t)ref[i].right - 5, mixed[i].right) << "Frame " << i;
    }
}

TEST_F(AudioDecoderMp3Test, AddSamplesFloatAccumulatesOntoExistingValues)
{
    // Referenz: linear als Float dekodieren, nativ also ohne die 16-Bit-Zwischenstufe
    pplib::File refFile;
    pplib::AudioDecoder_MP3 refDec;
    Open(refFile, refDec, "test_192cbr.mp3");
    std::vector<pplib::STEREOSAMPLE_FLOAT> ref;
    DecodeAllFloat(refDec, ref);

    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_192cbr.mp3");

    pplib::STEREOSAMPLE_FLOAT mixed[512];
    for (size_t i = 0; i < 512; i++) {
        mixed[i].left = 0.25f;
        mixed[i].right = -0.25f;
    }
    ASSERT_EQ((size_t)512, dec.addSamples(512, mixed));
    for (size_t i = 0; i < 512; i++) {
        EXPECT_NEAR(ref[i].left + 0.25f, mixed[i].left, 1e-6f) << "Frame " << i;
        EXPECT_NEAR(ref[i].right - 0.25f, mixed[i].right, 1e-6f) << "Frame " << i;
    }
}

TEST_F(AudioDecoderMp3Test, SeekLandsOnExactFrame)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_192cbr.mp3");

    // linear uebersprungene Frames als Referenz
    pplib::STEREOSAMPLE16 skip[8];
    size_t left = 5000;
    while (left) {
        size_t want = (left < 8) ? left : 8;
        const size_t got = dec.getSamples(want, skip);
        ASSERT_NE((size_t)0, got);
        left -= got;
    }
    std::vector<pplib::STEREOSAMPLE16> ref(64);
    ASSERT_EQ((size_t)64, dec.getSamples(64, ref.data()));

    dec.seekSample(5000);
    EXPECT_EQ((size_t)5000, dec.getPosition());
    std::vector<pplib::STEREOSAMPLE16> got(64);
    ASSERT_EQ((size_t)64, dec.getSamples(64, got.data()));
    Compare16(ref, got, 5000);
}

TEST_F(AudioDecoderMp3Test, SeekBackwardsToZeroReproducesFirstFrames)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_192cbr.mp3");

    std::vector<pplib::STEREOSAMPLE16> first(64);
    ASSERT_EQ((size_t)64, dec.getSamples(64, first.data()));

    pplib::STEREOSAMPLE16 skip[1009];
    size_t left = 3000;
    while (left) {
        size_t want = (left < 1009) ? left : 1009;
        const size_t got = dec.getSamples(want, skip);
        ASSERT_NE((size_t)0, got);
        left -= got;
    }

    dec.seekSample(0);
    EXPECT_EQ((size_t)0, dec.getPosition());
    std::vector<pplib::STEREOSAMPLE16> again(64);
    ASSERT_EQ((size_t)64, dec.getSamples(64, again.data()));
    Compare16(first, again, 0);
}

TEST_F(AudioDecoderMp3Test, MonoAndVbrSeekMatchLinearDecode)
{
    struct sCase
    {
        const char* file;
    };
    const sCase cases[] = {{"test_22khz_mono.mp3"}, {"test_192vbr.mp3"}};
    for (const sCase& c : cases) {
        pplib::File linFile;
        pplib::AudioDecoder_MP3 linear;
        Open(linFile, linear, c.file);
        std::vector<pplib::STEREOSAMPLE16> ref(32);
        pplib::STEREOSAMPLE16 skip[32];
        size_t left = 5000;
        while (left) {
            const size_t want = (left < 32) ? left : 32;
            const size_t got = linear.getSamples(want, skip);
            ASSERT_NE((size_t)0, got) << c.file;
            left -= got;
        }
        ASSERT_EQ((size_t)32, linear.getSamples(32, ref.data())) << c.file;

        pplib::File seekFile;
        pplib::AudioDecoder_MP3 seekDec;
        Open(seekFile, seekDec, c.file);
        seekDec.seekSample(5000);
        std::vector<pplib::STEREOSAMPLE16> got(32);
        ASSERT_EQ((size_t)32, seekDec.getSamples(32, got.data())) << c.file;
        Compare16(ref, got, 5000);
    }
}

TEST_F(AudioDecoderMp3Test, SeekBeyondLastFrameYieldsNoSamples)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_192cbr.mp3");

    dec.seekSample(1000000);
    pplib::STEREOSAMPLE16 buf[16];
    EXPECT_EQ((size_t)0, dec.getSamples(16, buf));
    EXPECT_EQ((size_t)0, dec.getSamples(16, buf));
}

TEST_F(AudioDecoderMp3Test, ReadingAtEndKeepsReturningZero)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_192cbr.mp3");

    std::vector<pplib::STEREOSAMPLE16> all;
    DecodeAll16(dec, all);
    const size_t frames = all.size();
    ASSERT_LT((size_t)170000, frames);
    EXPECT_EQ(frames, dec.getPosition());

    pplib::STEREOSAMPLE16 buf[16];
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ((size_t)0, dec.getSamples(16, buf));
    }
    pplib::STEREOSAMPLE_FLOAT fbuf[16];
    EXPECT_EQ((size_t)0, dec.getSamples(16, fbuf));
    EXPECT_EQ(frames, dec.getPosition());
}

TEST_F(AudioDecoderMp3Test, CloseResetsInfoAllowsDoubleCloseAndReopen)
{
    pplib::File file;
    OpenTestFile(file, "test_192cbr.mp3");

    pplib::AudioDecoder_MP3 dec;
    ASSERT_NO_THROW(dec.open(file));
    ASSERT_NO_THROW(dec.close());
    ASSERT_NO_THROW(dec.close());
    EXPECT_EQ(pplib::AudioInfo::AudioFormat::UNKNOWN, dec.getAudioInfo().Format);
    EXPECT_EQ((uint64_t)0, dec.getAudioInfo().Channels);
    EXPECT_EQ((uint64_t)0, dec.getAudioInfo().Samples);
    EXPECT_EQ((size_t)0, dec.getPosition());

    // Dasselbe Objekt fuer eine andere Datei verwenden
    pplib::File otherFile;
    OpenTestFile(otherFile, "test_32khz_stereo.mp3");
    ASSERT_NO_THROW(dec.open(otherFile));
    EXPECT_EQ((uint64_t)2, dec.getAudioInfo().Channels);
    EXPECT_EQ((uint64_t)32000, dec.getAudioInfo().Frequency);
    pplib::STEREOSAMPLE16 buf[16];
    EXPECT_EQ((size_t)16, dec.getSamples(16, buf));
}

TEST_F(AudioDecoderMp3Test, FloatSamplesStayOnNormalizedScale)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_192cbr.mp3");

    std::vector<pplib::STEREOSAMPLE_FLOAT> all;
    DecodeAllFloat(dec, all);
    ASSERT_LT((size_t)170000, all.size());
    float peak = 0.0f;
    for (size_t i = 0; i < all.size(); i++) {
        ASSERT_LE(std::fabs(all[i].left), 1.0f) << "Frame " << i;
        ASSERT_LE(std::fabs(all[i].right), 1.0f) << "Frame " << i;
        peak = std::max(peak, std::max(std::fabs(all[i].left), std::fabs(all[i].right)));
    }
    EXPECT_LT(0.1f, peak);
}

TEST_F(AudioDecoderMp3Test, ClosedDecoderRejectsEveryReadEntry)
{
    pplib::AudioDecoder_MP3 dec; // zustand: angelegt, aber keine Spur geoeffnet
    pplib::STEREOSAMPLE16 out16[8];
    pplib::STEREOSAMPLE32 mix[8] = {};
    pplib::STEREOSAMPLE_FLOAT outFloat[8];
    EXPECT_EQ((size_t)0, dec.getSamples(8, out16));
    EXPECT_EQ((size_t)0, dec.addSamples(8, mix));
    EXPECT_EQ((size_t)0, dec.getSamples(8, outFloat));
    EXPECT_EQ((size_t)0, dec.addSamples(8, outFloat));
    EXPECT_THROW(dec.seekSample(0), pplib::DecoderException);
}

TEST_F(AudioDecoderMp3Test, ReadEntryPointsRejectMissingBuffer)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    Open(file, dec, "test_192cbr.mp3");
    EXPECT_EQ((size_t)0, dec.getSamples(8, static_cast<pplib::STEREOSAMPLE16*>(nullptr)));
    EXPECT_EQ((size_t)0, dec.addSamples(8, static_cast<pplib::STEREOSAMPLE32*>(nullptr)));
    EXPECT_EQ((size_t)0, dec.getSamples(8, static_cast<pplib::STEREOSAMPLE_FLOAT*>(nullptr)));
    EXPECT_EQ((size_t)0, dec.addSamples(8, static_cast<pplib::STEREOSAMPLE_FLOAT*>(nullptr)));
}

TEST_F(AudioDecoderMp3Test, OpenRejectsForeignFileFormat)
{
    pplib::File file;
    pplib::AudioDecoder_MP3 dec;
    OpenTestFile(file, "test_44kHz.wav");
    EXPECT_THROW(dec.open(file), pplib::UnsupportedAudioFormatException);
}

TEST_F(AudioDecoderMp3Test, OpenRejectsAudioInfoThatDoesNotFitTheStream)
{
    pplib::File file;
    OpenTestFile(file, "test_192cbr.mp3");
    pplib::AudioDecoder_MP3 dec;

    pplib::AudioInfo bogus;
    memset(&bogus, 0, sizeof(bogus));
    bogus.Format = pplib::AudioInfo::AudioFormat::MP3;
    // Channels ==  fuehrt in der alten Implementation zu einer Division durch Null
    EXPECT_THROW(dec.open(file, &bogus), pplib::UnsupportedAudioFormatException);

    bogus.Channels = 2;
    bogus.Frequency = 44100;
    bogus.AudioEnd = 100;
    bogus.AudioStart = 200;
    EXPECT_THROW(dec.open(file, &bogus), pplib::UnsupportedAudioFormatException);
    EXPECT_THROW(dec.seekSample(0), pplib::DecoderException);
    EXPECT_EQ((size_t)0, dec.getSamples(16, (pplib::STEREOSAMPLE16*)nullptr));
}

TEST_F(AudioDecoderMp3Test, ContentWithoutMp3FramesThrowsUnsupportedFormat)
{
    std::vector<unsigned char> junk(600, 'x');
    memcpy(junk.data(), "this is not an mp3 file", 21);
    pplib::MemFile mem(junk.data(), junk.size());

    pplib::AudioDecoder_MP3 dec;
    EXPECT_THROW(dec.open(mem), pplib::UnsupportedAudioFormatException);
}

TEST_F(AudioDecoderMp3Test, UnparsableFramesEndStreamWithoutSamples)
{
    std::vector<unsigned char> junk(600, 'x');
    pplib::MemFile mem(junk.data(), junk.size());

    pplib::AudioInfo bogus;
    memset(&bogus, 0, sizeof(bogus));
    bogus.Format = pplib::AudioInfo::AudioFormat::MP3;
    bogus.Channels = 2;
    bogus.Frequency = 44100;
    bogus.BitsPerSample = 16;
    bogus.BytesPerSample = 4;
    bogus.AudioStart = 0;
    bogus.AudioEnd = junk.size();

    pplib::AudioDecoder_MP3 dec;
    ASSERT_NO_THROW(dec.open(mem, &bogus));
    pplib::STEREOSAMPLE16 out[64];
    EXPECT_EQ(0u, dec.getSamples(64, out));
    EXPECT_EQ(0u, dec.getSamples(64, out));
    EXPECT_THROW(dec.seekSample(1000000000ull), pplib::DecoderException);
    EXPECT_NO_THROW(dec.close());
}

namespace
{
/** @brief Datenquelle, deren Positionierung wunschweise eine Exception wirft
 *
 * Simuliert eine abbrechende Quelle (Netzwerkstream, Pipe, gekappte Datei). Die Rueckruffunktionen
 * von libmpg123 laufen in fremdem C-Code; eine Exception darf dort nicht herauslaufen.
 */
class ThrowingMemFile : public pplib::MemFile
{
public:
    /// wirft beim absoluten Positionieren, wie es der Lesecallback vor jedem Leseversuch tut
    bool failAbsolute = false;
    /// wirft beim relativen Positionieren, das libmpg123 waehrend eines Suchlaufs nutzt
    bool failRelative = false;

    ThrowingMemFile(void* adresse, size_t size)
        : pplib::MemFile(adresse, size)
    {
    }

    void seek(uint64_t position) override
    {
        if (failAbsolute) throw pplib::EndOfFileException("injizierter Positionierungsfehler");
        pplib::MemFile::seek(position);
    }

    uint64_t seek(int64_t offset, pplib::FileObject::SeekOrigin origin) override
    {
        if (failRelative) throw pplib::EndOfFileException("injizierter Positionierungsfehler");
        return pplib::MemFile::seek(offset, origin);
    }
};
} // namespace

TEST_F(AudioDecoderMp3Test, ThrowingStreamStopsDecodeWithoutEscapingException)
{
    pplib::File file;
    OpenTestFile(file, "test_192cbr.mp3");
    std::vector<unsigned char> raw((size_t)file.size());
    ASSERT_EQ(raw.size(), file.read(raw.data(), raw.size()));
    file.close();

    ThrowingMemFile mem(raw.data(), raw.size());
    pplib::AudioDecoder_MP3 dec;
    ASSERT_NO_THROW(dec.open(mem));

    std::vector<pplib::STEREOSAMPLE16> first(3000);
    ASSERT_EQ((size_t)3000, dec.getSamples(3000, first.data()));

    mem.failAbsolute = true; // trifft den Lesecallback, der vor jedem Leseversuch positioniert
    // die Menge liegt ueber dem gefuellten Puffer, libmpg123 muss also nachlesen
    std::vector<pplib::STEREOSAMPLE16> more(9000);
    size_t got = 0;
    EXPECT_NO_THROW(got = dec.getSamples(9000, more.data()));
    EXPECT_LT(got, (size_t)9000);

    pplib::STEREOSAMPLE16 tail[64];
    EXPECT_NO_THROW(dec.getSamples(64, tail));
    EXPECT_NO_THROW(dec.close());
}

TEST_F(AudioDecoderMp3Test, ThrowingSeekCallbackRaisesDecoderException)
{
    pplib::File file;
    OpenTestFile(file, "test_192cbr.mp3");
    std::vector<unsigned char> raw((size_t)file.size());
    ASSERT_EQ(raw.size(), file.read(raw.data(), raw.size()));
    file.close();

    ThrowingMemFile mem(raw.data(), raw.size());
    pplib::AudioDecoder_MP3 dec;
    ASSERT_NO_THROW(dec.open(mem));

    std::vector<pplib::STEREOSAMPLE16> got(3000);
    EXPECT_EQ((size_t)3000, dec.getSamples(3000, got.data()));

    // Vor der Fehlereinspielung dekodiert, damit das Suchziel hinter der aktuellen Position liegt:
    // nur ein Rueckwaertssuchlauf zwingt libmpg123 in den Suchcallback. Der Callback fangs die
    // Exception ab und meldet -1; der Decoder meldet den fehlgeschlagenen Suchlauf.
    mem.failAbsolute = true;
    mem.failRelative = true;
    EXPECT_THROW(dec.seekSample(100), pplib::DecoderException);
    EXPECT_NO_THROW(dec.close());
}

TEST_F(AudioDecoderMp3Test, CorruptedFramesMidStreamStopDecoding)
{
    pplib::File file;
    OpenTestFile(file, "test_192cbr.mp3");
    std::vector<unsigned char> raw((size_t)file.size());
    ASSERT_EQ(raw.size(), file.read(raw.data(), raw.size()));
    file.close();
    // ab der Mitte des Datenblocks sind keine MPEG-Rahmen mehr zu finden
    memset(raw.data() + raw.size() / 2, 0, raw.size() / 2);

    pplib::MemFile mem(raw.data(), raw.size());
    pplib::AudioDecoder_MP3 dec;
    ASSERT_NO_THROW(dec.open(mem));
    std::vector<pplib::STEREOSAMPLE16> out;
    DecodeAll16(dec, out);
    EXPECT_GT(out.size(), (size_t)0);
}

#endif // HAVE_MPG123
