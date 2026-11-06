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

#if defined(__GLIBC__)
#include <malloc.h> // mallinfo2(): belegter Heap-Platz als Messgroesse fuer den Rohpuffer
#endif

#include <config_pplib.h>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/decoder.h>
#include <pplib/audio/sample_formats.h>
#include <pplib/core/file.h>
#include <pplib/core/functions.h>
#include <pplib/core/memfile.h>
#include <pplib/exceptions.h>

#ifdef HAVE_LIBFLAC

namespace
{

//!\brief Anzahl Frames pro Ladeschritt bei den Ganzdatei-Vergleichen
constexpr size_t kChunk = 1009;

#if defined(__GLIBC__)
/** @brief Belegter Heap-Platz, inklusive der mmapped Bloecke ab 128 kB
 */
static size_t LiveHeap()
{
    const struct mallinfo2 mi = mallinfo2();
    return mi.uordblks + mi.hblkhd;
}
#endif // __GLIBC__

/** @brief Dekodiert den Rest der Datei in einen Vektor
 *
 *  Der Schritt von 1009 Frames ist bewusst keine Vielfacher von typischen FLAC-
 *  Blockgroessen (z.B. 4096/8192), damit Pufferuebergaenge und das Nachlesen
 *  quer ueber Rahmengrenzen geprüft werden.
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

static void DecodeAll32(pplib::AudioDecoder& dec, std::vector<pplib::STEREOSAMPLE32>& out)
{
    out.clear();
    pplib::STEREOSAMPLE32 buf[kChunk];
    for (;;) {
        memset(buf, 0, sizeof(buf));
        const size_t got = dec.addSamples(kChunk, buf);
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

} // namespace

//!\brief Basis-Fixture fuer AudioDecoder_Flac
class AudioDecoderFlacTest : public ::testing::Test
{
protected:
    /** @brief Oeffnet eine echte Testdatei unter tests/testdata/audio */
    static void OpenTestFile(pplib::File& file, const char* name)
    {
        const std::string path = std::string("testdata/audio/") + name;
        ASSERT_NO_THROW(file.open(path.c_str())) << "open() warf fuer " << path;
        ASSERT_TRUE(file.isOpen()) << "Datei ist nicht geoeffnet: " << path;
    }

    /** @brief Oeffnet den Flac-Decoder fuer eine Testdatei */
    void Open(pplib::File& file, pplib::AudioDecoder_Flac& dec, const char* name)
    {
        OpenTestFile(file, name);
        ASSERT_NO_THROW(dec.open(file));
    }
};

TEST_F(AudioDecoderFlacTest, OpenFillsAudioInfoAndFactorySelectsFlac)
{
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit.flac");

    pplib::AudioInfo info = dec.getAudioInfo();
    EXPECT_EQ(pplib::AudioInfo::AudioFormat::FLAC, info.Format);
    EXPECT_EQ((uint64_t)2, info.Channels);
    EXPECT_EQ((uint64_t)16, info.BitsPerSample);
    EXPECT_EQ(pplib::AudioInfo::SampleEncoding::INTEGER, info.SampleType);
    EXPECT_EQ((uint64_t)44100, info.Frequency);
    EXPECT_EQ((uint64_t)173695, info.Samples);
    EXPECT_EQ((uint64_t)3938, info.Length);
    EXPECT_EQ((uint64_t)86, info.AudioStart);

    // Der Fabrik-Decoder muss dasselbe Format liefern
    pplib::File file2;
    OpenTestFile(file2, "test_44kHz_16bit.flac");
    pplib::AudioDecoder* made = pplib::GetAudioDecoder(file2);
    ASSERT_NE(nullptr, made);
    EXPECT_NE(nullptr, dynamic_cast<pplib::AudioDecoder_Flac*>(made));
    EXPECT_EQ((uint64_t)173695, made->getAudioInfo().Samples);
    delete made;
}

TEST_F(AudioDecoderFlacTest, Samples16MatchWaveDecoder)
{
    // Die 16-Bit-FLAC-Datei enthaelt bitgenau dieselben Samples wie test_44kHz.wav
    pplib::File waveFile;
    OpenTestFile(waveFile, "test_44kHz.wav");
    pplib::AudioDecoder_Wave wave;
    ASSERT_NO_THROW(wave.open(waveFile));
    std::vector<pplib::STEREOSAMPLE16> ref;
    DecodeAll16(wave, ref);
    ASSERT_EQ((size_t)173695, ref.size());

    pplib::File flacFile;
    pplib::AudioDecoder_Flac flac;
    Open(flacFile, flac, "test_44kHz_16bit.flac");
    std::vector<pplib::STEREOSAMPLE16> got;
    DecodeAll16(flac, got);

    ASSERT_EQ(ref.size(), got.size());
    for (size_t i = 0; i < ref.size(); i++) {
        ASSERT_EQ(ref[i].left, got[i].left) << "Frame " << i;
        ASSERT_EQ(ref[i].right, got[i].right) << "Frame " << i;
    }
}

TEST_F(AudioDecoderFlacTest, Samples32MatchWaveDecoder)
{
    pplib::File waveFile;
    OpenTestFile(waveFile, "test_44kHz.wav");
    pplib::AudioDecoder_Wave wave;
    ASSERT_NO_THROW(wave.open(waveFile));
    std::vector<pplib::STEREOSAMPLE32> ref;
    DecodeAll32(wave, ref);
    ASSERT_EQ((size_t)173695, ref.size());

    pplib::File flacFile;
    pplib::AudioDecoder_Flac flac;
    Open(flacFile, flac, "test_44kHz_16bit.flac");
    std::vector<pplib::STEREOSAMPLE32> got;
    DecodeAll32(flac, got);

    ASSERT_EQ(ref.size(), got.size());
    for (size_t i = 0; i < ref.size(); i++) {
        ASSERT_EQ(ref[i].left, got[i].left) << "Frame " << i;
        ASSERT_EQ(ref[i].right, got[i].right) << "Frame " << i;
    }
}

TEST_F(AudioDecoderFlacTest, ReadIntoTargetOverwritesPrefilledBuffer)
{
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit.flac");

    pplib::STEREOSAMPLE16 buf[4];
    for (size_t i = 0; i < 4; i++) {
        buf[i].left = 0x1234;
        buf[i].right = -0x1234;
    }
    ASSERT_EQ((size_t)4, dec.getSamples(4, buf));

    // getSamples() weist zu, addSamples() addiert. Ein frischer Decoder liefert zum Vergleich
    // dieselben Werte, ohne dass der vorgefuellte Puffer eine Rolle spielt.
    pplib::STEREOSAMPLE16 nach[4];
    pplib::File vergleichsDatei;
    pplib::AudioDecoder_Flac vergleich;
    Open(vergleichsDatei, vergleich, "test_44kHz_16bit.flac");
    ASSERT_EQ((size_t)4, vergleich.getSamples(4, nach));
    for (size_t i = 0; i < 4; i++) {
        EXPECT_EQ(nach[i].left, buf[i].left) << "Frame " << i;
        EXPECT_EQ(nach[i].right, buf[i].right) << "Frame " << i;
    }
}

TEST_F(AudioDecoderFlacTest, MemFileDecodesLikeARealFile)
{
    // Alle Betriebs-Callbacks laufen ueber ein Objekt ohne echten Dateiverweis
    std::vector<char> data = ReadWholeFile("test_44kHz_16bit.flac");
    ASSERT_FALSE(data.empty());

    pplib::MemFile mem(data.data(), data.size());
    ASSERT_TRUE(mem.isOpen());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(mem, info));
    ASSERT_EQ(pplib::AudioInfo::AudioFormat::FLAC, info.Format);

    pplib::AudioDecoder_Flac dec;
    ASSERT_NO_THROW(dec.open(mem));
    EXPECT_EQ((uint64_t)2, dec.getAudioInfo().Channels);
    EXPECT_EQ((uint64_t)173695, dec.getAudioInfo().Samples);

    pplib::File waveFile;
    OpenTestFile(waveFile, "test_44kHz.wav");
    pplib::AudioDecoder_Wave wave;
    ASSERT_NO_THROW(wave.open(waveFile));
    std::vector<pplib::STEREOSAMPLE16> ref;
    DecodeAll16(wave, ref);
    ASSERT_EQ((size_t)173695, ref.size());

    std::vector<pplib::STEREOSAMPLE16> got;
    DecodeAll16(dec, got);
    ASSERT_EQ(ref.size(), got.size());
    for (size_t i = 0; i < ref.size(); i += 997) {
        ASSERT_EQ(ref[i].left, got[i].left) << "Frame " << i;
        ASSERT_EQ(ref[i].right, got[i].right) << "Frame " << i;
    }

    // Ein Sprung innerhalb der Speicherdatei muss dieselbe Stelle treffen wie in der echten Datei
    ASSERT_NO_THROW(dec.seekSample(100000));
    EXPECT_EQ((size_t)100000, dec.getPosition());
    pplib::STEREOSAMPLE16 buf[8];
    ASSERT_EQ((size_t)8, dec.getSamples(8, buf));
    for (size_t i = 0; i < 8; i++) {
        EXPECT_EQ(ref[100000 + i].left, buf[i].left) << "Frame " << (100000 + i);
        EXPECT_EQ(ref[100000 + i].right, buf[i].right) << "Frame " << (100000 + i);
    }
}

TEST_F(AudioDecoderFlacTest, TwoDecodersShareOneFileObjectWithoutCorruptingFrames)
{
    // Die Doku empfiehlt exklusive Nutzung des FileObjects. Geteilte Nutzung fuehrt nachweislich
    // nicht zu kaputten Frames: Beide Decoder setzen vor jeder Dateiaktion ihre eigene Position
    // (open() den Streamanfang, fillFrames() die gesicherte Position am Trackende). Wer am selben
    // Objekt lesen oder schreiben will, kann sich aber nicht auf den Dateimarker verlassen.
    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit.flac");

    pplib::File refFile;
    OpenTestFile(refFile, "test_44kHz.wav");
    pplib::AudioDecoder_Wave wave;
    ASSERT_NO_THROW(wave.open(refFile));
    std::vector<pplib::STEREOSAMPLE16> erwartet;
    DecodeAll16(wave, erwartet);
    ASSERT_EQ((size_t)173695, erwartet.size());

    pplib::AudioDecoder_Flac erster;
    ASSERT_NO_THROW(erster.open(file));
    pplib::STEREOSAMPLE16 buf[kChunk];
    ASSERT_EQ(kChunk, erster.getSamples(kChunk, buf));

    // Zweiter Decoder offnet und liest am selben Objekt, waehrend der erste mitten in der Kette sitzt
    pplib::AudioDecoder_Flac zweiter;
    ASSERT_NO_THROW(zweiter.open(file));
    ASSERT_EQ(kChunk, zweiter.getSamples(kChunk, buf));

    // Der erste Decoder macht an Frame kChunk weiter und liefert dieselben Frames wie ein
    // Decoder mit eigenem FileObject
    pplib::File vergleichsDatei;
    OpenTestFile(vergleichsDatei, "test_44kHz_16bit.flac");
    pplib::AudioDecoder_Flac vergleich;
    ASSERT_NO_THROW(vergleich.open(vergleichsDatei));
    std::vector<pplib::STEREOSAMPLE16> kette;
    DecodeAll16(vergleich, kette);
    ASSERT_EQ(erwartet.size(), kette.size());

    ASSERT_EQ(kChunk, erster.getSamples(kChunk, buf));
    for (size_t i = 0; i < kChunk; i++) {
        EXPECT_EQ(kette[kChunk + i].left, buf[i].left) << "Frame " << (kChunk + i);
        EXPECT_EQ(kette[kChunk + i].right, buf[i].right) << "Frame " << (kChunk + i);
    }

    // Der Dateimarker liegt nach den Dekodierlaeufen nicht mehr am Audiodatenanfang (Byte 86 der Datei)
    EXPECT_GT(file.tell(), (int64_t)86);
}

TEST_F(AudioDecoderFlacTest, FloatSamplesMatchWaveDecoder)
{
    pplib::File waveFile;
    OpenTestFile(waveFile, "test_44kHz.wav");
    pplib::AudioDecoder_Wave wave;
    ASSERT_NO_THROW(wave.open(waveFile));
    std::vector<pplib::STEREOSAMPLE_FLOAT> ref;
    DecodeAllFloat(wave, ref);
    ASSERT_EQ((size_t)173695, ref.size());

    pplib::File flacFile;
    pplib::AudioDecoder_Flac flac;
    Open(flacFile, flac, "test_44kHz_16bit.flac");
    std::vector<pplib::STEREOSAMPLE_FLOAT> got;
    DecodeAllFloat(flac, got);

    ASSERT_EQ(ref.size(), got.size());
    for (size_t i = 0; i < ref.size(); i++) {
        ASSERT_FLOAT_EQ(ref[i].left, got[i].left) << "Frame " << i;
        ASSERT_FLOAT_EQ(ref[i].right, got[i].right) << "Frame " << i;
    }
    // 16-Bit-Float normiert auf 32768
    EXPECT_NEAR(452 / 32768.0f, got[0].left, 1e-6f);
}

TEST_F(AudioDecoderFlacTest, Samples24ScaleIntegerTargetsByShift)
{
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_24bit.flac");
    EXPECT_EQ((uint64_t)24, dec.getAudioInfo().BitsPerSample);

    // Float liefert den vollen 24-Bit-Wert, die Ganzzahl-Ziele den um 8 Bits
    // verschobenen Wert. Ueber die Float-Skala wird der Rohwert zurueckgerechnet.
    pplib::STEREOSAMPLE16 i16[512];
    pplib::STEREOSAMPLE_FLOAT fl[512];
    ASSERT_EQ((size_t)512, dec.getSamples(512, fl));
    dec.seekSample(0);
    ASSERT_EQ((size_t)512, dec.getSamples(512, i16));
    for (size_t i = 0; i < 512; i++) {
        const int32_t rawL = (int32_t)lrintf(fl[i].left * 8388608.0f);
        const int32_t rawR = (int32_t)lrintf(fl[i].right * 8388608.0f);
        EXPECT_EQ((int16_t)(rawL >> 8), i16[i].left) << "Frame " << i;
        EXPECT_EQ((int16_t)(rawR >> 8), i16[i].right) << "Frame " << i;
        EXPECT_TRUE(rawL >= -8388608 && rawL <= 8388607) << "Frame " << i;
    }
}

TEST_F(AudioDecoderFlacTest, Samples24MatchTheReferenceDecoder)
{
    // Die 24-Bit-Kette hat keinen passenden WAV-Bezug im Testdatensatz (test_44kHz_24bit.wav
    // unterscheidet sich in vielen Frames um 1 LSB). Die Anker stammen vom Referenz-Decoder
    // "flac -d" auf dieselbe Datei, rechtsbuendig auf 16 Bit gebracht (>>8) wie hier ueblich.
    // Frame 173694 ist der letzte der Kette.
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_24bit.flac");

    struct Anker
    {
        uint64_t frame;
        int16_t left;
        int16_t right;
    };
    static const Anker anker[] = {{0, 452, 434},    {1, 676, 643},       {2, 590, 572},      {3, 626, 621},      {4, 599, 587},
                                  {5, 627, 612},    {10000, 1577, -503}, {10001, 2506, 300}, {10002, 3254, 803}, {10003, 3619, 1083},
                                  {173679, -1, -5}, {173680, 0, 7},      {173681, 2, -6},    {173682, -5, 5},    {173683, 8, -4},
                                  {173684, -10, 3}, {173692, 2, -1},     {173693, -4, 0},    {173694, 7, 0}};

    pplib::STEREOSAMPLE16 buf[8];
    for (const Anker& a : anker) {
        ASSERT_NO_THROW(dec.seekSample(a.frame));
        ASSERT_LE((size_t)1, dec.getSamples(1, buf)) << "Frame " << a.frame;
        EXPECT_EQ(a.left, buf[0].left) << "Frame " << a.frame;
        EXPECT_EQ(a.right, buf[0].right) << "Frame " << a.frame;
    }
}

TEST_F(AudioDecoderFlacTest, MonoFileReplicatesTheOnlyChannel)
{
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_mono.flac");
    EXPECT_EQ((uint64_t)1, dec.getAudioInfo().Channels);
    EXPECT_EQ((uint64_t)16, dec.getAudioInfo().BitsPerSample);
    EXPECT_EQ((uint64_t)173695, dec.getAudioInfo().Samples);

    pplib::STEREOSAMPLE16 buf[256];
    ASSERT_EQ((size_t)256, dec.getSamples(256, buf));
    bool nonzero = false;
    for (size_t i = 0; i < 256; i++) {
        ASSERT_EQ(buf[i].left, buf[i].right) << "Frame " << i;
        if (buf[i].left != 0) nonzero = true;
    }
    EXPECT_TRUE(nonzero) << "Mono-Samples sollten nicht durchgaengig 0 sein";
}

/** @brief alle vier Zielvarianten an einer Monodatei: Duplizat und dieselbe Skalierung
 *
 *  Jede Lesemethode fuehrt eine eigene Schleife; geprueft wird, dass alle aus demselben
 *  Rohpuffer dieselben Werte liefern und links wie rechts dasselbe steht.
 */
TEST_F(AudioDecoderFlacTest, MonoFileFeedsEveryTargetLoopWithDuplicatedChannel)
{
    std::vector<pplib::STEREOSAMPLE16> s16;
    {
        pplib::File file;
        pplib::AudioDecoder_Flac dec;
        Open(file, dec, "test_44kHz_mono.flac");
        DecodeAll16(dec, s16);
    }
    ASSERT_EQ((size_t)173695, s16.size());

    std::vector<pplib::STEREOSAMPLE32> mix;
    {
        pplib::File file;
        pplib::AudioDecoder_Flac dec;
        Open(file, dec, "test_44kHz_mono.flac");
        DecodeAll32(dec, mix);
    }

    std::vector<pplib::STEREOSAMPLE_FLOAT> flt;
    {
        pplib::File file;
        pplib::AudioDecoder_Flac dec;
        Open(file, dec, "test_44kHz_mono.flac");
        DecodeAllFloat(dec, flt);
    }

    ASSERT_EQ(s16.size(), mix.size());
    ASSERT_EQ(s16.size(), flt.size());

    bool nonzero = false;
    for (size_t i = 0; i < s16.size(); i++) {
        EXPECT_EQ(s16[i].left, s16[i].right) << "Frame " << i;
        EXPECT_EQ((int32_t)s16[i].left, mix[i].left) << "Frame " << i;
        EXPECT_EQ((int32_t)s16[i].right, mix[i].right) << "Frame " << i;
        EXPECT_FLOAT_EQ((float)s16[i].left / 32768.0f, flt[i].left) << "Frame " << i;
        if (s16[i].left != 0) nonzero = true;
    }
    EXPECT_TRUE(nonzero) << "Mono-Samples sollten nicht durchgaengig 0 sein";
}

TEST_F(AudioDecoderFlacTest, SeekLandsOnExactSample)
{
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit.flac");

    std::vector<pplib::STEREOSAMPLE16> ref;
    DecodeAll16(dec, ref);
    ASSERT_EQ((size_t)173695, ref.size());

    // Werte vor, auf und hinter typischen Blockgrenzen sowie in der Naehe des Endes
    const size_t targets[] = {1, 3, 4095, 4096, 4097, 5000, 102400, 173693};
    for (size_t target : targets) {
        dec.seekSample(target);
        EXPECT_EQ(target, dec.getPosition());
        pplib::STEREOSAMPLE16 one[2];
        ASSERT_EQ((size_t)2, dec.getSamples(2, one)) << "target " << target;
        EXPECT_EQ(ref[target].left, one[0].left) << "target " << target;
        EXPECT_EQ(ref[target].right, one[0].right) << "target " << target;
        EXPECT_EQ(ref[target + 1].left, one[1].left) << "target " << target;
    }
}

TEST_F(AudioDecoderFlacTest, SeekBackwardsAndRepeatedSeekSameTarget)
{
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit.flac");

    std::vector<pplib::STEREOSAMPLE16> ref;
    DecodeAll16(dec, ref);

    dec.seekSample(120000);
    pplib::STEREOSAMPLE16 a[100];
    ASSERT_EQ((size_t)100, dec.getSamples(100, a));
    dec.seekSample(42);
    ASSERT_EQ((size_t)42, dec.getPosition());
    pplib::STEREOSAMPLE16 b[100];
    ASSERT_EQ((size_t)100, dec.getSamples(100, b));
    for (size_t i = 0; i < 100; i++) {
        EXPECT_EQ(ref[42 + i].left, b[i].left) << "Frame " << i;
        EXPECT_EQ(ref[42 + i].right, b[i].right) << "Frame " << i;
    }
    // Zweiter Sprung auf dasselbe Ziel: Position bleibt, Inhalt identisch
    dec.seekSample(42);
    pplib::STEREOSAMPLE16 c[10];
    ASSERT_EQ((size_t)10, dec.getSamples(10, c));
    EXPECT_EQ(ref[42].left, c[0].left);
    EXPECT_EQ(ref[51].right, c[9].right);
}

TEST_F(AudioDecoderFlacTest, StreamedFileWithoutSampleCountDecodesAndSkipsLinearly)
{
    // Eine ueber eine Pipe kodierte Datei nennt im STREAMINFO 0 als Gesamtsamplezahl.
    // libFLAC kann deshalb nicht springen; seekSample() initialisiert neu und lies von vorn
    // bis zum Ziel. Die Pipe-Kodierung nutzt ausserdem ein grosses Padding-Block.
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit_stream.flac");
    EXPECT_EQ((uint64_t)0, dec.getAudioInfo().Samples);
    EXPECT_EQ((uint64_t)0, dec.getAudioInfo().Length);

    pplib::File refFile;
    pplib::AudioDecoder_Flac ref;
    Open(refFile, ref, "test_44kHz_16bit.flac");

    pplib::STEREOSAMPLE16 a[8];
    pplib::STEREOSAMPLE16 b[8];
    ASSERT_EQ((size_t)8, ref.getSamples(8, a));
    ASSERT_EQ((size_t)8, dec.getSamples(8, b));
    for (size_t i = 0; i < 8; i++) {
        ASSERT_EQ(a[i].left, b[i].left) << "Frame " << i;
        ASSERT_EQ(a[i].right, b[i].right) << "Frame " << i;
    }
    EXPECT_EQ((size_t)8, dec.getPosition());

    // Der Sprung fuehrt ueber den sequenziellen Pfad, das Ziel muss trotzdem exakt sitzen
    dec.seekSample(100000);
    ref.seekSample(100000);
    EXPECT_EQ((size_t)100000, dec.getPosition());
    ASSERT_EQ((size_t)4, dec.getSamples(4, a));
    ASSERT_EQ((size_t)4, ref.getSamples(4, b));
    for (size_t i = 0; i < 4; i++) {
        ASSERT_EQ(a[i].left, b[i].left) << "gesuchter Frame " << i;
        ASSERT_EQ(a[i].right, b[i].right) << "gesuchter Frame " << i;
    }

    // Ende ist das wirkliche Streamende, nicht die fehlende Samplezahl
    std::vector<pplib::STEREOSAMPLE16> rest;
    DecodeAll16(dec, rest);
    EXPECT_EQ((size_t)73691, rest.size()) << "alle Rahmen bis zum Dateiende";
    EXPECT_EQ((size_t)173695, dec.getPosition());
}

TEST_F(AudioDecoderFlacTest, LinearFallbackSeekDoesNotBufferTheJumpDistance)
{
    // Ohne Gesamtsamplezahl im STREAMINFO kann libFLAC nicht springen: seekSample() liesst
    // vom Datenanfang bis zum Ziel vor. Der Rohpuffer darf mit der Sprungdistanz nicht
    // wachsen, sonst kostet eine Stunde Material gut einen Gigabyte.
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit_stream.flac");
    ASSERT_EQ((uint64_t)0, dec.getAudioInfo().Samples);

    pplib::STEREOSAMPLE16 probe[8];
    ASSERT_EQ((size_t)8, dec.getSamples(8, probe));
#if defined(__GLIBC__)
    const size_t heapBefore = LiveHeap();
#endif
    dec.seekSample(170000);
    EXPECT_EQ((size_t)170000, dec.getPosition());
#if defined(__GLIBC__)
    const size_t heapAfter = LiveHeap();
    // 170 000 Frames waeren 1,36 MB Rohpuffer; in Portionen gelesen bleiben es Dutzende KB
    EXPECT_LT(heapAfter, heapBefore + 300 * 1024) << "Rohpuffer wuchs mit der Sprungdistanz um " << (heapAfter - heapBefore) << " Byte";
#endif

    // Der lineare Fallback muss trotzdem exakt am Ziel sitzen
    pplib::STEREOSAMPLE16 got[8];
    ASSERT_EQ((size_t)8, dec.getSamples(8, got));
    pplib::File refFile;
    pplib::AudioDecoder_Flac ref;
    Open(refFile, ref, "test_44kHz_16bit.flac");
    ref.seekSample(170000);
    pplib::STEREOSAMPLE16 want[8];
    ASSERT_EQ((size_t)8, ref.getSamples(8, want));
    for (size_t i = 0; i < 8; i++) {
        EXPECT_EQ(want[i].left, got[i].left) << "Frame " << i;
        EXPECT_EQ(want[i].right, got[i].right) << "Frame " << i;
    }
}

TEST_F(AudioDecoderFlacTest, SeekBeyondEndClampsToLastSample)
{
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit.flac");

    std::vector<pplib::STEREOSAMPLE16> ref;
    DecodeAll16(dec, ref);
    ASSERT_EQ((size_t)173695, ref.size());

    dec.seekSample(200000);
    EXPECT_EQ((size_t)173694, dec.getPosition());
    pplib::STEREOSAMPLE16 one[4];
    ASSERT_EQ((size_t)1, dec.getSamples(4, one));
    EXPECT_EQ(ref[173694].left, one[0].left);
    EXPECT_EQ(ref[173694].right, one[0].right);
    EXPECT_EQ((size_t)0, dec.getSamples(4, one));
}

TEST_F(AudioDecoderFlacTest, ReadingAtEndKeepsReturningZero)
{
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit.flac");

    std::vector<pplib::STEREOSAMPLE16> ref;
    DecodeAll16(dec, ref);
    EXPECT_EQ((size_t)173695, dec.getPosition());

    pplib::STEREOSAMPLE16 buf[16];
    pplib::STEREOSAMPLE_FLOAT fbuf[16];
    EXPECT_EQ((size_t)0, dec.getSamples(16, buf));
    EXPECT_EQ((size_t)0, dec.getSamples(16, buf));
    pplib::STEREOSAMPLE32 ibuf[16];
    EXPECT_EQ((size_t)0, dec.addSamples(16, ibuf));
    EXPECT_EQ((size_t)0, dec.getSamples(16, fbuf));
    EXPECT_EQ((size_t)173695, dec.getPosition());

    // Sprung zurueck hebt das Ende wieder auf
    dec.seekSample(0);
    EXPECT_EQ((size_t)16, dec.getSamples(16, buf));
}

TEST_F(AudioDecoderFlacTest, AddSamplesAccumulatesOntoExistingValues)
{
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit.flac");

    pplib::STEREOSAMPLE32 ibuf[64];
    for (size_t i = 0; i < 64; i++) {
        ibuf[i].left = 1000;
        ibuf[i].right = -1000;
    }
    ASSERT_EQ((size_t)64, dec.addSamples(64, ibuf));
    pplib::STEREOSAMPLE16 ref[64];
    dec.seekSample(0);
    ASSERT_EQ((size_t)64, dec.getSamples(64, ref));
    EXPECT_EQ((int32_t)ref[10].left + 1000, ibuf[10].left);
    EXPECT_EQ((int32_t)ref[10].right - 1000, ibuf[10].right);

    pplib::STEREOSAMPLE_FLOAT fbuf[64];
    for (size_t i = 0; i < 64; i++) {
        fbuf[i].left = 0.5f;
        fbuf[i].right = -0.5f;
    }
    dec.seekSample(0);
    ASSERT_EQ((size_t)64, dec.addSamples(64, fbuf));
    EXPECT_NEAR(ref[5].left / 32768.0f + 0.5f, fbuf[5].left, 1e-6f);
    EXPECT_NEAR(ref[5].right / 32768.0f - 0.5f, fbuf[5].right, 1e-6f);
}

TEST_F(AudioDecoderFlacTest, TaggedAndSeektableFilesDecodeLikePlainFile)
{
    pplib::File plainFile;
    pplib::AudioDecoder_Flac plain;
    Open(plainFile, plain, "test_44kHz_16bit.flac");
    std::vector<pplib::STEREOSAMPLE16> ref;
    DecodeAll16(plain, ref);
    ASSERT_EQ((size_t)173695, ref.size());

    const char* variants[] = {"test_44kHz_16bit_seektable.flac", "test_44kHz_16bit_id3v2.flac", "test_44kHz_16bit_id3v1.flac"};
    for (const char* name : variants) {
        pplib::File file;
        pplib::AudioDecoder_Flac dec;
        ASSERT_NO_THROW(Open(file, dec, name)) << name;
        EXPECT_EQ((uint64_t)173695, dec.getAudioInfo().Samples) << name;
        std::vector<pplib::STEREOSAMPLE16> got;
        DecodeAll16(dec, got);
        ASSERT_EQ(ref.size(), got.size()) << name;
        for (size_t i = 0; i < ref.size(); i++) {
            ASSERT_EQ(ref[i].left, got[i].left) << name << " Frame " << i;
            ASSERT_EQ(ref[i].right, got[i].right) << name << " Frame " << i;
        }
        // Nach dem kompletten Durchlesen darf kein Fehlerzustand zurueckbleiben
        dec.seekSample(0);
        pplib::STEREOSAMPLE16 one[2];
        EXPECT_EQ((size_t)2, dec.getSamples(2, one)) << name;
    }
}

TEST_F(AudioDecoderFlacTest, TruncatedFileYieldsPrefixOfIntactFile)
{
    pplib::File intactFile;
    pplib::AudioDecoder_Flac intact;
    Open(intactFile, intact, "test_44kHz_16bit.flac");
    // Die schnittscharfe Datei enthaelt zwei komplette Rahmen zu je 4096 Rahmen
    constexpr size_t kRef = 8192;
    pplib::STEREOSAMPLE16 ref[kRef];
    ASSERT_EQ(kRef, intact.getSamples(kRef, ref));

    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit_truncated.flac");
    // Das AudioInfo nennt die deklarierte Samplezahl der abgeschnittenen Datei
    EXPECT_EQ((uint64_t)173695, dec.getAudioInfo().Samples);

    std::vector<pplib::STEREOSAMPLE16> got;
    DecodeAll16(dec, got);
    EXPECT_LT(got.size(), (size_t)173695);
    EXPECT_EQ((size_t)8192, got.size()) << "zwei vollstaendige Rahmen mussten lesbar sein";
    const size_t cmp = got.size() < kRef ? got.size() : kRef;
    for (size_t i = 0; i < cmp; i++) {
        ASSERT_EQ(ref[i].left, got[i].left) << "Frame " << i;
        ASSERT_EQ(ref[i].right, got[i].right) << "Frame " << i;
    }
}

TEST_F(AudioDecoderFlacTest, TruncatedFileSeeksFallBackToLinearSkip)
{
    // Der Sniffer uebernimmt die Samplezahl des originalen Kopfs, das Material endet aber
    // frueher. Ein Sprung ueber das reale Ende hinaus muss fehlschlagen, ohne zu werfen:
    // der Decoder liest von vorn und bleibt am verfuegbaren Ende stehen.
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit_truncated.flac");

    std::vector<pplib::STEREOSAMPLE16> got;
    DecodeAll16(dec, got);
    const size_t available = got.size();
    ASSERT_GT(available, (size_t)4000);

    dec.seekSample(170000);
    EXPECT_LE(dec.getPosition(), available);
    pplib::STEREOSAMPLE16 buf[256];
    EXPECT_EQ((size_t)0, dec.getSamples(256, buf));

    // zurueck in den verfuegbaren Bereich: Werte stimmen mit dem intakten File ueberein
    dec.seekSample(2000);
    ASSERT_EQ((size_t)2000, dec.getPosition());
    pplib::File fullFile;
    pplib::AudioDecoder_Flac full;
    Open(fullFile, full, "test_44kHz_16bit.flac");
    full.seekSample(2000);
    pplib::STEREOSAMPLE16 a[4], b[4];
    ASSERT_EQ((size_t)4, dec.getSamples(4, a));
    ASSERT_EQ((size_t)4, full.getSamples(4, b));
    for (size_t i = 0; i < 4; i++) {
        ASSERT_EQ(a[i].left, b[i].left) << "Frame " << 2000 + i;
        ASSERT_EQ(a[i].right, b[i].right) << "Frame " << 2000 + i;
    }
}

TEST_F(AudioDecoderFlacTest, ForeignAudioInfoWithoutFlacDataThrows)
{
    // Ein AudioInfo, das FLAC vorgaukelt, aber zu keinem_headerlosen Material passt,
    // muss abgewiesen werden, obwohl alle Felder fuer sich gueltig sind.
    pplib::AudioInfo src;
    src.Format = pplib::AudioInfo::AudioFormat::FLAC;
    src.FileSize = 13;
    src.AudioStart = 0;
    src.AudioSize = 13;
    src.Channels = 2;
    src.Samples = 1;
    src.BitsPerSample = 16;
    src.BytesPerSample = 4;
    src.Frequency = 44100;
    src.SampleType = pplib::AudioInfo::SampleEncoding::INTEGER;

    pplib::File junk;
    junk.open("testdata/compression.txt");
    ASSERT_TRUE(junk.isOpen());
    pplib::AudioDecoder_Flac dec;
    EXPECT_THROW(dec.open(junk, &src), pplib::DecoderException);

    const uint8_t bytes[13] = {'N', 'i', 'c', 'h', 't', 'f', 'l', 'a', 'c', 'W', 'X', 'Y', 'Z'};
    pplib::MemFile mem((void*)bytes, sizeof(bytes));
    pplib::AudioDecoder_Flac memDec;
    EXPECT_THROW(memDec.open(mem, &src), pplib::DecoderException);

    // Reines "fLaC"-Magic ohne STREAMINFO-Block: der Sniffer der Bibliothek lehnt ab
    const uint8_t head[4] = {'f', 'L', 'a', 'C'};
    pplib::MemFile magicOnly((void*)head, sizeof(head));
    pplib::AudioDecoder_Flac headDec;
    EXPECT_THROW(headDec.open(magicOnly, &src), pplib::DecoderException);

    // STREAMINFO-Block angekuedigt, aber unvollstaendig bzw. unbrauchbar befuellt:
    // libFLAC bricht beim lesen der Metadaten ab
    uint8_t broken[48] = {};
    memcpy(broken, "fLaC", 4);
    broken[4] = 0x80; // letzter Block, Typ STREAMINFO
    broken[5] = 0x00;
    broken[6] = 0x00;
    broken[7] = 0x22; // 34 BytePayload
    pplib::MemFile brokenMeta((void*)broken, sizeof(broken));
    pplib::AudioDecoder_Flac brokenDec;
    EXPECT_THROW(brokenDec.open(brokenMeta, &src), pplib::UnsupportedAudioFormatException);
    // Metadaten-Block mit ungueltiger Laenge (STREAMINFO muss 34 Bytes haben): Metadaten-Parsing bricht ab
    static uint8_t badLenBuf[44] = {'f', 'L', 'a', 'C', 0x80, 0x00, 0x7F, 0xFF};
    pplib::MemFile badLen(badLenBuf, sizeof(badLenBuf));
    EXPECT_THROW(brokenDec.open(badLen, &src), pplib::DecoderException);
    // Nach einem fehlgeschlagenen open() ist der Decoder leer und kann neu geoeffnet werden
    pplib::File again;
    OpenTestFile(again, "test_44kHz_16bit.flac");
    ASSERT_NO_THROW(brokenDec.open(again));
    brokenDec.close();
    EXPECT_EQ(pplib::AudioInfo::AudioFormat::UNKNOWN, headDec.getAudioInfo().Format);
}

TEST_F(AudioDecoderFlacTest, FlacChainWithoutStreamInfoIsRejected)
{
    // Eine Kette aus nur PADDING-Blocken fuehrt keinen STREAMINFO-Block. Gibt der Aufrufer
    // ein passendes AudioInfo mit, umgeht er die Datei-Erkennung; open() muss trotzdem
    // abbrechen und darf die libFLAC-Instanz zurckgeben.
    std::vector<uint8_t> bytes;
    bytes.push_back('f');
    bytes.push_back('L');
    bytes.push_back('a');
    bytes.push_back('C');
    bytes.push_back(0x01); // PADDING, nicht der letzte Block
    bytes.push_back(0x00);
    bytes.push_back(0x00);
    bytes.push_back(0x28); // 40 Byte Payload
    bytes.insert(bytes.end(), 40, 0);
    bytes.push_back(0x81); // letzter Block, wieder PADDING
    bytes.push_back(0x00);
    bytes.push_back(0x00);
    bytes.push_back(0x00);
    ASSERT_EQ((size_t)52, bytes.size());

    pplib::AudioInfo src;
    src.Format = pplib::AudioInfo::AudioFormat::FLAC;
    src.FileSize = bytes.size();
    src.AudioStart = 0;
    src.AudioSize = bytes.size();
    src.Channels = 2;
    src.Samples = 100;
    src.BitsPerSample = 16;
    src.BytesPerSample = 4;
    src.Frequency = 44100;
    src.SampleType = pplib::AudioInfo::SampleEncoding::INTEGER;

    pplib::MemFile mem(bytes.data(), bytes.size());
    pplib::AudioDecoder_Flac dec;
    EXPECT_THROW(dec.open(mem, &src), pplib::DecoderException);
    EXPECT_EQ(pplib::AudioInfo::AudioFormat::UNKNOWN, dec.getAudioInfo().Format);
}

TEST_F(AudioDecoderFlacTest, EveryOpenRoundReleasesTheDecoderInstance)
{
    // open()/close() und jeder abgelehnte open()-Versuch muss die libFLAC-Instanz
    // wieder freigeben, sonst bleiben die Decodier-Puffer des Decoders verloren.
    // Der Test prueft das Verhalten; die Freigabe selbst sieht nur ein Leak-Sanitizer.
    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit.flac");
    pplib::File monoFile;
    OpenTestFile(monoFile, "test_44kHz_mono.flac");
    pplib::AudioInfo mono;
    ASSERT_TRUE(pplib::IdentAudioFile(monoFile, mono));
    ASSERT_EQ((uint64_t)1, mono.Channels);

    pplib::AudioDecoder_Flac dec;
    pplib::STEREOSAMPLE16 buf[8];
    pplib::STEREOSAMPLE16 first[8];
    for (int round = 0; round < 3; round++) {
        ASSERT_NO_THROW(dec.open(file));
        ASSERT_EQ((size_t)8, dec.getSamples(8, buf));
        if (round == 0) {
            memcpy(first, buf, sizeof(first));
        } else {
            EXPECT_EQ(first[0].left, buf[0].left) << "Runde " << round;
            EXPECT_EQ(first[1].right, buf[1].right) << "Runde " << round;
        }
        dec.close();
        // Format und Bitbreite passen zum mitgegebenen AudioInfo, die Kanalzahl nicht:
        // Der Abbruch erfolgt erst nach der Initialisierung des libFLAC-Decoders
        EXPECT_THROW(dec.open(file, &mono), pplib::UnsupportedAudioFormatException);
    }

    ASSERT_NO_THROW(dec.open(file));
    EXPECT_EQ((size_t)173695, dec.getAudioInfo().Samples);
    ASSERT_EQ((size_t)8, dec.getSamples(8, buf));
    EXPECT_EQ(first[0].left, buf[0].left);
    dec.close();
}

TEST_F(AudioDecoderFlacTest, CorruptAudioMaterialStopsWithoutHanging)
{
    // Zeroes-Material mitten im Stream: libFLAC verliert den Rahmentakt und sucht
    // den naechsten Sync. Der Decoder darf dabei weder hängen noch etwas werfen.
    pplib::File file;
    file.open("testdata/audio/test_44kHz_16bit.flac");
    ASSERT_TRUE(file.isOpen());
    std::vector<uint8_t> bytes((size_t)file.size());
    ASSERT_EQ(bytes.size(), (size_t)file.read(bytes.data(), bytes.size()));
    file.close();
    // 512 Byte mitten in den Audiodaten zerstoeren
    memset(&bytes[150000], 0xFF, 512);

    pplib::MemFile mem(bytes.data(), bytes.size());
    pplib::AudioDecoder_Flac dec;
    dec.open(mem);

    pplib::File refFile;
    pplib::AudioDecoder_Flac ref;
    Open(refFile, ref, "test_44kHz_16bit.flac");

    std::vector<pplib::STEREOSAMPLE16> good, corrupt;
    DecodeAll16(ref, good);
    ASSERT_EQ((size_t)173695, good.size());

    // Bis zum Fehler wird geliefert, ab dem Fehler liefert der Decoder 0
    DecodeAll16(dec, corrupt);
    // Bis zur zerstoerten Stelle dekodiert der Decoder weiter. Die zerstoerten Frames fehlen in
    // der Kette, deshalb ist die Gesamtzahl kleiner als im unbeschaedigten Material.
    EXPECT_EQ((size_t)77824, corrupt.size());
    for (size_t i = 0; i < 4000; i++) {
        ASSERT_EQ(good[i].left, corrupt[i].left) << "Frame " << i;
        ASSERT_EQ(good[i].right, corrupt[i].right) << "Frame " << i;
    }
    pplib::STEREOSAMPLE16 buf[8];
    EXPECT_EQ((size_t)0, dec.getSamples(8, buf));
    // Ein Sprung hebt den Fehler auf, danach dekodiert der Decoder wieder
    dec.seekSample(100);
    ASSERT_EQ((size_t)8, dec.getSamples(8, buf));
    EXPECT_EQ(good[100].left, buf[0].left);
}

TEST_F(AudioDecoderFlacTest, ClosedFileAbortsDecodingAtTheCallbacks)
{
    // Wird die Datei mitten im Betrieb geschlossen, werfen tell()/size()/read() eine
    // Exception. Die Betriebs-Callbacks duerfen sie nicht durch den C-Stack von libFLAC
    // reichen lassen, sondern melden sie als Abbruch; geworfen wird erst in der Lesemethode.
    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit.flac");
    pplib::AudioDecoder_Flac dec;
    ASSERT_NO_THROW(dec.open(file));

    pplib::STEREOSAMPLE16 buf[1024];
    size_t before = 0;
    while (before < 20000) {
        const size_t got = dec.getSamples(1024, buf);
        if (got == 0) break;
        before += got;
    }
    ASSERT_GT(before, (size_t)10000);
    file.close();

    // Der Eingabepuffer von libFLAC haelt noch einige Bloecke: spaetestens nach dem
    // Nachfuellen aus der Datei stoppt das Lesen und liefert 0
    size_t after = 0;
    size_t got;
    while (after < 150000 && (got = dec.getSamples(1024, buf)) > 0) {
        after += got;
    }
    EXPECT_LT(after, (size_t)20000);

    // Ein Sprung ueber die geschlossene Datei liefert bei der naechsten Leseanfrage 0
    dec.seekSample(100000);
    EXPECT_EQ((size_t)0, dec.getSamples(64, buf));
    dec.close();
}

TEST_F(AudioDecoderFlacTest, SeekAfterReopeningTheFileStillDetectsTheEnd)
{
    // Ein Dateifehler in einem Betriebs-Callback setzt ioError, damit ein durch den
    // Abbruch entstehendes "Streamende" nicht als echtes Dateiende gilt. Bleibt das
    // Merkmal nach einem erfolgreichen Sprung stehen, kann fillFrames()
    // sein Ende-Flag nicht mehr setzen: weder atEnd noch error wird wahr, und die
    // Nachleseschleife laeuft endlos.
    const char* name = "test_44kHz_16bit.flac";
    const std::string path = std::string("testdata/audio/") + name;
    pplib::File file;
    OpenTestFile(file, name);
    pplib::AudioDecoder_Flac dec;
    ASSERT_NO_THROW(dec.open(file));

    pplib::STEREOSAMPLE16 buf[1024];
    size_t got = 0;
    file.close();
    // Der Lese-Callback von libFLAC holt Portionsgroessen aus der Datei: erst wenn der
    // interne Eingabepuffer leer ist, stoppt das Lesen und liefert 0
    while (got < 40000) {
        const size_t n = dec.getSamples(1024, buf);
        if (n == 0) break;
        got += n;
    }
    EXPECT_LT(got, (size_t)20000);

    // Dieselbe FileObject-Instanz wieder oeffnen: damit gelingt der naechste Sprung
    ASSERT_NO_THROW(file.open(path.c_str()));
    ASSERT_TRUE(file.isOpen());
    dec.seekSample(40000);
    ASSERT_EQ((size_t)40000, dec.getPosition());

    // Ab hier entscheidet sich, ob die Nachleseschleife ihr Ende findet: ohne das
    // Zuruecksetzen von ioError dreht sie unendlich (nachweisbar per Aussen-Timeout)
    std::vector<pplib::STEREOSAMPLE16> out;
    DecodeAll16(dec, out);
    EXPECT_EQ((size_t)(173695 - 40000), out.size());
    EXPECT_EQ((size_t)173695, dec.getPosition());
    dec.close();
}

TEST_F(AudioDecoderFlacTest, SecondPassOverCorruptMaterialIsJudgedAlike)
{
    // Die Fehlerunterdriickung nahe dem Sample-Ende darf nicht von der Decode-Historie
    // abhaengen: dieselbe fehlerhafte Stelle muss in jedem Durchlauf als Fehler enden.
    pplib::File file;
    file.open("testdata/audio/test_44kHz_16bit.flac");
    ASSERT_TRUE(file.isOpen());
    std::vector<uint8_t> bytes((size_t)file.size());
    ASSERT_EQ(bytes.size(), (size_t)file.read(bytes.data(), bytes.size()));
    file.close();
    memset(&bytes[150000], 0xFF, 512);

    pplib::MemFile mem(bytes.data(), bytes.size());
    pplib::AudioDecoder_Flac dec;
    dec.open(mem);

    pplib::File refFile;
    pplib::AudioDecoder_Flac ref;
    Open(refFile, ref, "test_44kHz_16bit.flac");
    std::vector<pplib::STEREOSAMPLE16> good;
    DecodeAll16(ref, good);
    ASSERT_EQ((size_t)173695, good.size());

    std::vector<pplib::STEREOSAMPLE16> first;
    // Der Frame-Zaehler laeuft ueber alle Durchlaeufe weiter, bis er die SAMPLE-Gesamtzahl
    // uebersteigt: ab dann wirke die Unterdriickung auch bei echtem Datenfehler
    for (int pass = 1; pass <= 3; pass++) {
        std::vector<pplib::STEREOSAMPLE16> out;
        DecodeAll16(dec, out);
        EXPECT_GT(out.size(), (size_t)4000) << "Durchlauf " << pass;
        EXPECT_LT(out.size(), good.size()) << "Durchlauf " << pass;
        if (pass == 1) {
            first = out;
            for (size_t i = 0; i < 4000; i++) {
                ASSERT_EQ(good[i].left, out[i].left) << "Durchlauf 1 Frame " << i;
                ASSERT_EQ(good[i].right, out[i].right) << "Durchlauf 1 Frame " << i;
            }
        } else {
            ASSERT_EQ(first.size(), out.size()) << "Durchlauf " << pass;
            for (size_t i = 0; i < first.size(); i++) {
                ASSERT_EQ(first[i].left, out[i].left) << "Durchlauf " << pass << " Frame " << i;
                ASSERT_EQ(first[i].right, out[i].right) << "Durchlauf " << pass << " Frame " << i;
            }
        }
        dec.seekSample(0);
    }
}

TEST_F(AudioDecoderFlacTest, CloseInvalidatesInfoAndRejectsFurtherReads)
{
    pplib::File file;
    pplib::AudioDecoder_Flac dec;
    Open(file, dec, "test_44kHz_16bit.flac");
    dec.seekSample(1000);
    dec.close();
    EXPECT_EQ(pplib::AudioInfo::AudioFormat::UNKNOWN, dec.getAudioInfo().Format);
    EXPECT_EQ((uint64_t)0, dec.getAudioInfo().Samples);
    EXPECT_EQ((size_t)0, dec.getPosition());

    pplib::STEREOSAMPLE16 buf[8];
    pplib::STEREOSAMPLE32 ibuf[8];
    pplib::STEREOSAMPLE_FLOAT fbuf[8];
    EXPECT_EQ((size_t)0, dec.getSamples(8, buf));
    EXPECT_EQ((size_t)0, dec.addSamples(8, ibuf));
    EXPECT_EQ((size_t)0, dec.getSamples(8, fbuf));
    // Ein Sprung auf dem geschlossenen Decoder ist wirkungslos, wirft aber nicht
    dec.seekSample(10);
    EXPECT_EQ((size_t)0, dec.getPosition());
    // Nochmal schliessen ist ungefaehrlich
    dec.close();

    // Wieder oeffnen funktioniert und liefert neue Werte
    pplib::File second;
    OpenTestFile(second, "test_44kHz_24bit.flac");
    ASSERT_NO_THROW(dec.open(second));
    EXPECT_EQ((uint64_t)24, dec.getAudioInfo().BitsPerSample);
    EXPECT_EQ((size_t)8, dec.getSamples(8, buf));
}

TEST_F(AudioDecoderFlacTest, OpenRejectsUnsupportedFlacAndForeignFiles)
{
    // 8 Bit und 6 Kanaele: guiltiges FLAC, das die Erkennung auch meldet, aber ohne Decoder
    const char* rejected[] = {"test_44kHz_8bit.flac", "test_44kHz_51.flac"};
    for (const char* name : rejected) {
        pplib::File file;
        OpenTestFile(file, name);
        pplib::AudioDecoder_Flac dec;
        EXPECT_THROW(dec.open(file), pplib::UnsupportedAudioFormatException) << name;
        // mit uebergebenem AudioInfo greift dieselbe Ablehnung
        pplib::AudioInfo info;
        ASSERT_TRUE(pplib::IdentAudioFile(file, info)) << name;
        EXPECT_EQ(pplib::AudioInfo::AudioFormat::FLAC, info.Format) << name;
        pplib::AudioDecoder_Flac withInfo;
        EXPECT_THROW(withInfo.open(file, &info), pplib::UnsupportedAudioFormatException) << name;
    }

    // Eine WAV-Datei wird von der Erkennung sauber erkannt, bleibt aber fremd
    pplib::File wavFile;
    OpenTestFile(wavFile, "test_44kHz.wav");
    pplib::AudioInfo wavInfo;
    ASSERT_TRUE(pplib::IdentAudioFile(wavFile, wavInfo));
    EXPECT_EQ(pplib::AudioInfo::AudioFormat::WAVE, wavInfo.Format);
    pplib::AudioDecoder_Flac wavDec;
    EXPECT_THROW(wavDec.open(wavFile), pplib::UnsupportedAudioFormatException);
    // Ein Puffer ohne ID3-Tag und ohne fLaC-Magic ist kein FLAC-Stream
    std::vector<char> junk = {'n', 'o', 'p', 'e', '', '', 'n', 'o', 'p', 'e'};
    pplib::MemFile mem(junk.data(), junk.size());
    pplib::AudioDecoder_Flac dec;
    EXPECT_THROW(dec.open(mem), pplib::UnsupportedAudioFormatException);
}

TEST_F(AudioDecoderFlacTest, OpenRejectsAudioInfoThatDoesNotFitTheStream)
{
    // AudioInfo einer WAV-Datei: falsches Format
    pplib::File waveFile;
    OpenTestFile(waveFile, "test_44kHz.wav");
    pplib::AudioInfo waveInfo;
    ASSERT_TRUE(pplib::IdentAudioFile(waveFile, waveInfo));

    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit.flac");
    pplib::AudioDecoder_Flac dec;
    EXPECT_THROW(dec.open(file, &waveInfo), pplib::UnsupportedAudioFormatException);

    //richtiges Format, aber vorgetaeuschte 32 Bit
    pplib::AudioInfo bogus = dec.getAudioInfo();
    bogus.Format = pplib::AudioInfo::AudioFormat::FLAC;
    bogus.BitsPerSample = 32;
    EXPECT_THROW(dec.open(file, &bogus), pplib::UnsupportedAudioFormatException);

    //richtiges Format und Aufloesung, aber falsche Kanalzahl gegen STREAMINFO
    pplib::AudioInfo mismatch = dec.getAudioInfo();
    mismatch.Format = pplib::AudioInfo::AudioFormat::FLAC;
    mismatch.Channels = 1;
    EXPECT_THROW(dec.open(file, &mismatch), pplib::UnsupportedAudioFormatException);

    //AudioInfo einer Mono-Datei passt nicht zum Stereo-Strom
    pplib::AudioInfo mono;
    pplib::File monoFile;
    OpenTestFile(monoFile, "test_44kHz_mono.flac");
    ASSERT_TRUE(pplib::IdentAudioFile(monoFile, mono));
    EXPECT_EQ((uint64_t)1, mono.Channels);
    EXPECT_THROW(dec.open(file, &mono), pplib::UnsupportedAudioFormatException);

    //Das passend ermittelte AudioInfo derselben Datei ist der normale Weg
    pplib::AudioInfo same;
    ASSERT_TRUE(pplib::IdentAudioFile(file, same));
    ASSERT_NO_THROW(dec.open(file, &same));
    EXPECT_EQ((uint64_t)173695, dec.getAudioInfo().Samples);
}

TEST_F(AudioDecoderFlacTest, FactoryReturnsNullWhenTheFileCannotBeOpened)
{
    // Gueltiges FLAC mit abgelehnter Bitbreite: open() wirft. Die Fabrik soll
    // NULL zurueckgeben statt die Exception durchzureichen.
    pplib::File file;
    OpenTestFile(file, "test_44kHz_8bit.flac");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    ASSERT_EQ(pplib::AudioInfo::AudioFormat::FLAC, info.Format);

    pplib::AudioDecoder* dec = NULL;
    ASSERT_NO_THROW(dec = pplib::GetAudioDecoder(file));
    EXPECT_EQ(NULL, dec);
    if (dec) delete dec;
}

TEST_F(AudioDecoderFlacTest, SmallestPlayableFlacIsAccepted)
{
    // Kuerzest spielbare FLAC-Datei: Kopf bis zum Ende des letzten Metadata-Blocks plus ein
    // Byte Audiodaten. Der Sniffer erkennt so eine Datei (AudioSize=1); der Decoder durfte sie
    // nicht mit der pauschalen 44-Byte-Pruefung ablehnen.
    pplib::File full;
    OpenTestFile(full, "test_44kHz_16bit.flac");
    full.seek(0, pplib::File::SEEKSET);
    std::vector<char> head(43, (char)0xFF);
    ASSERT_EQ((size_t)42, full.read(head.data(), 42));
    head[4] = (char)0x80; // letzte Metadata-Kappe: STREAMINFO (Typ 0, 34 Bytes) schliesst ab

    pplib::MemFile mem(head.data(), head.size());
    ASSERT_TRUE(mem.isOpen());
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(mem, info));
    ASSERT_EQ(pplib::AudioInfo::AudioFormat::FLAC, info.Format);
    ASSERT_EQ((uint64_t)1, info.AudioSize);

    pplib::AudioDecoder_Flac dec;
    // Die Datei wird nicht mehr als "zu kurz" abgewiesen. Der eine Rest-Byte enthaelt keinen
    // gueltigen Rahmen, also liefert der Decoder keine Samples.
    EXPECT_NO_THROW(dec.open(mem));
    pplib::STEREOSAMPLE16 out[4];
    EXPECT_EQ((uint64_t)0, dec.getSamples(4, out));
}

#else
// Build ohne libFLAC: Der Konstruktor liefert ein gueltiges, aber leeres Objekt; alle
// Methoden ausser den beiden Konstanzengettern müssen UnsupportedFeatureException werfen.
// Es werden dieselben echten Testdateien benutzt wie im Build mit libFLAC.

//!\brief Anzahl angeforderter Frames in den Lesetests
constexpr size_t kStubFrames = 16;

//!\brief Basis-Fixture fuer den Build ohne libFLAC
class AudioDecoderFlacStubTest : public ::testing::Test
{
protected:
    /** @brief Oeffnet eine echte Testdatei unter tests/testdata/audio */
    static void OpenTestFile(pplib::File& file, const char* name)
    {
        const std::string path = std::string("testdata/audio/") + name;
        ASSERT_NO_THROW(file.open(path.c_str())) << "open() warf fuer " << path;
        ASSERT_TRUE(file.isOpen()) << "Datei ist nicht geoeffnet: " << path;
    }
};

TEST_F(AudioDecoderFlacStubTest, ConstructorAndDestructorDoNotThrow)
{
    // Das Objekt ist konstruierbar und wieder zerstoerbar, ohne zu werfen
    EXPECT_NO_THROW({ pplib::AudioDecoder_Flac dec; });
}

TEST_F(AudioDecoderFlacStubTest, NewDecoderReportsEmptyAudioInfo)
{
    pplib::AudioDecoder_Flac dec;
    const pplib::AudioInfo info = dec.getAudioInfo();
    EXPECT_EQ(pplib::AudioInfo::AudioFormat::UNKNOWN, info.Format);
    EXPECT_EQ((uint64_t)0, info.Samples);
    EXPECT_EQ((uint8_t)0, info.Channels);
    EXPECT_EQ((uint64_t)0, dec.getPosition());
}

TEST_F(AudioDecoderFlacStubTest, OpenThrowsWithoutFlac)
{
    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit.flac");
    pplib::AudioDecoder_Flac dec;
    EXPECT_THROW(dec.open(file), pplib::UnsupportedFeatureException);
    // Zweiter Versuch ueber den optionalen AudioInfo-Parameter wirft ebenfalls
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_THROW(dec.open(file, &info), pplib::UnsupportedFeatureException);
}

TEST_F(AudioDecoderFlacStubTest, CloseThrowsWithoutFlac)
{
    pplib::AudioDecoder_Flac dec;
    EXPECT_THROW(dec.close(), pplib::UnsupportedFeatureException);
}

TEST_F(AudioDecoderFlacStubTest, SeekSampleThrowsWithoutFlac)
{
    pplib::AudioDecoder_Flac dec;
    EXPECT_THROW(dec.seekSample(0), pplib::UnsupportedFeatureException);
}

TEST_F(AudioDecoderFlacStubTest, ReaderMethodsReturnZeroWithoutFlac)
{
    pplib::AudioDecoder_Flac dec;
    pplib::STEREOSAMPLE16 buf16[kStubFrames];
    pplib::STEREOSAMPLE32 buf32[kStubFrames];
    pplib::STEREOSAMPLE_FLOAT bufFloat[kStubFrames];
    EXPECT_EQ((size_t)0, dec.getSamples(kStubFrames, buf16));
    EXPECT_EQ((size_t)0, dec.addSamples(kStubFrames, buf32));
    EXPECT_EQ((size_t)0, dec.getSamples(kStubFrames, bufFloat));
    EXPECT_EQ((size_t)0, dec.addSamples(kStubFrames, bufFloat));
}

TEST_F(AudioDecoderFlacStubTest, FactoryReturnsNullForFlacFile)
{
    // Die Dateierkennung ist Bibliotheks-unabhaengig, der Decoder steht nicht zur Verfuegung:
    // Die Fabrik faengt die Exception ab und liefert NULL.
    pplib::File file;
    OpenTestFile(file, "test_44kHz_16bit.flac");
    pplib::AudioInfo info;
    ASSERT_TRUE(pplib::IdentAudioFile(file, info));
    EXPECT_EQ(pplib::AudioInfo::AudioFormat::FLAC, info.Format);
    pplib::AudioDecoder* made = pplib::GetAudioDecoder(file);
    EXPECT_EQ(nullptr, made);
}

#endif // HAVE_LIBFLAC