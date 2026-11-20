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

#include "OggVorbisHelper.h"
#include <string.h>
#include <array>
#include <pplib/core/functions.h>

namespace pplib
{

namespace
{

constexpr std::array<uint32_t, 256> makeOggCrcTable()
{
    std::array<uint32_t, 256> table{};
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t r = i << 24;
        for (int j = 0; j < 8; ++j) {
            if (r & 0x80000000U) {
                r = (r << 1) ^ 0x04C11DB7U;
            } else {
                r <<= 1;
            }
        }
        table[i] = r;
    }
    return table;
}

constexpr std::array<uint32_t, 256> kOggCrcTable = makeOggCrcTable();

} // namespace

uint32_t OggCrc32Update(uint32_t crc, const void* data, size_t size)
{
    const uint8_t* p = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) {
        crc = (crc << 8) ^ kOggCrcTable[((crc >> 24) & 0xFFU) ^ p[i]];
    }
    return crc;
}

uint32_t OggCrc32(const void* data, size_t size)
{
    return OggCrc32Update(0, data, size);
}

bool ReadOggPageHeader(FileObject& file, uint64_t offset, uint64_t fileSize, OggPageHeader& outHeader,
                       std::vector<uint8_t>* outSegments)
{
    if (offset + 27 > fileSize) return false;
    const char* hdr = file.tryMap(offset, 27);
    if (!hdr || memcmp(hdr, "OggS", 4) != 0) return false;

    const uint8_t version = Peek8(hdr + 4);
    if (version != 0) return false;

    const uint8_t numSegments = Peek8(hdr + 26);
    const uint32_t headerSize = 27U + (uint32_t)numSegments;
    if (offset + headerSize > fileSize) return false;

    const char* segPtr = (numSegments > 0) ? file.tryMap(offset + 27, numSegments) : nullptr;
    if (numSegments > 0 && !segPtr) return false;

    uint32_t payloadSize = 0;
    if (outSegments) outSegments->resize(numSegments);
    for (uint8_t i = 0; i < numSegments; ++i) {
        const uint8_t seg = Peek8(segPtr + i);
        payloadSize += seg;
        if (outSegments) (*outSegments)[i] = seg;
    }

    if (offset + headerSize + payloadSize > fileSize) return false;

    outHeader.version = version;
    outHeader.headerType = Peek8(hdr + 5);
    outHeader.granulePos = (int64_t)Peek64(hdr + 6);
    outHeader.serialNumber = Peek32(hdr + 14);
    outHeader.sequenceNumber = Peek32(hdr + 18);
    outHeader.crc = Peek32(hdr + 22);
    outHeader.numSegments = numSegments;
    outHeader.headerSize = headerSize;
    outHeader.payloadSize = payloadSize;
    return true;
}

bool ReadOggVorbisHeaders(FileObject& file, OggVorbisHeaders& out, bool needSetupPacket)
{
    const uint64_t fsize = file.size();
    uint64_t offset = 0;
    const int targetPackets = needSetupPacket ? 3 : 2;
    int packetIndex = 0;
    bool haveSerial = false;
    std::vector<uint8_t> currentPacket;
    std::vector<uint8_t> segments;

    out = OggVorbisHeaders{};

    while (packetIndex < targetPackets && offset < fsize) {
        OggPageHeader hdr;
        if (!ReadOggPageHeader(file, offset, fsize, hdr, &segments)) return false;

        if (!haveSerial) {
            if ((hdr.headerType & 0x02) == 0) return false; // BOS fehlt auf erster Page
            out.serialNumber = hdr.serialNumber;
            haveSerial = true;
        } else {
            if (hdr.serialNumber != out.serialNumber) return false;
        }

        const char* payload = (hdr.payloadSize > 0) ? file.tryMap(offset + hdr.headerSize, hdr.payloadSize) : nullptr;
        if (hdr.payloadSize > 0 && !payload) return false;

        size_t payloadPos = 0;
        for (uint8_t segLen : segments) {
            if (segLen > 0) {
                const uint8_t* p = reinterpret_cast<const uint8_t*>(payload + payloadPos);
                currentPacket.insert(currentPacket.end(), p, p + segLen);
                payloadPos += segLen;
            }
            if (segLen < 255) {
                // Paket vollstaendig
                if (packetIndex == 0) {
                    if (currentPacket.size() < 30) return false;
                    if (currentPacket[0] != 0x01 || memcmp(currentPacket.data() + 1, "vorbis", 6) != 0) return false;
                    const char* id = reinterpret_cast<const char*>(currentPacket.data());
                    const uint32_t vorbisVersion = Peek32(id + 7);
                    if (vorbisVersion != 0) return false;
                    out.channels = Peek8(id + 11);
                    out.frequency = Peek32(id + 12);
                    out.bitrateMax = (int32_t)Peek32(id + 16);
                    out.bitrateNominal = (int32_t)Peek32(id + 20);
                    out.bitrateMin = (int32_t)Peek32(id + 24);
                    if (out.channels == 0 || out.frequency == 0) return false;
                    if ((Peek8(id + 29) & 0x01) == 0) return false; // Framing-Bit muss 1 sein
                    out.idPacket.copy(currentPacket.data(), currentPacket.size());
                } else if (packetIndex == 1) {
                    if (currentPacket.size() < 7 + 8) return false;
                    if (currentPacket[0] != 0x03 || memcmp(currentPacket.data() + 1, "vorbis", 6) != 0) return false;
                    out.commentPacket.copy(currentPacket.data(), currentPacket.size());
                } else if (packetIndex == 2) {
                    if (currentPacket.size() < 7) return false;
                    if (currentPacket[0] != 0x05 || memcmp(currentPacket.data() + 1, "vorbis", 6) != 0) return false;
                    out.setupPacket.copy(currentPacket.data(), currentPacket.size());
                }
                currentPacket.clear();
                ++packetIndex;
                if (packetIndex >= targetPackets) break;
            }
        }

        offset += hdr.headerSize + hdr.payloadSize;
        out.audioPageStart = offset;
        out.nextSequenceNumber = hdr.sequenceNumber + 1;
    }

    return packetIndex == targetPackets;
}

bool FindLastOggGranulePos(FileObject& file, uint32_t serialNumber, uint64_t minOffset, int64_t& outGranulePos,
                           uint64_t& outLastPageEnd)
{
    const uint64_t fsize = file.size();
    if (fsize < minOffset + 27) return false;

    const uint64_t chunkSize = 65536;
    const uint64_t overlap = 65536; // max Ogg-Page passt komplett in die Ueberlappung
    uint64_t windowEnd = fsize;

    while (windowEnd > minOffset) {
        const uint64_t windowStart = (windowEnd - minOffset > chunkSize) ? (windowEnd - chunkSize) : minOffset;
        const size_t windowSize = (size_t)(windowEnd - windowStart);
        const char* buf = file.tryMap(windowStart, windowSize);
        if (!buf) return false;

        for (size_t pos = windowSize; pos >= 27; --pos) {
            const size_t idx = pos - 27;
            if (memcmp(buf + idx, "OggS", 4) != 0) continue;

            const uint64_t pageOffset = windowStart + idx;
            OggPageHeader hdr;
            if (!ReadOggPageHeader(file, pageOffset, fsize, hdr)) continue;
            if (hdr.serialNumber != serialNumber) continue;
            if (hdr.granulePos < 0) continue;

            // CRC pruefen, um zufaellige "OggS"-Sequenzen in Audiodaten auszuschliessen
            const size_t totalPageSize = (size_t)hdr.headerSize + (size_t)hdr.payloadSize;
            const char* pagePtr = file.tryMap(pageOffset, totalPageSize);
            if (!pagePtr) continue;
            std::vector<uint8_t> tmp(pagePtr, pagePtr + totalPageSize);
            tmp[22] = 0;
            tmp[23] = 0;
            tmp[24] = 0;
            tmp[25] = 0;
            if (OggCrc32(tmp.data(), tmp.size()) != hdr.crc) continue;

            outGranulePos = hdr.granulePos;
            outLastPageEnd = pageOffset + totalPageSize;
            return true;
        }

        if (windowStart == minOffset) break;
        windowEnd = (windowStart + overlap < fsize) ? (windowStart + overlap) : windowStart;
        if (windowEnd >= windowStart + chunkSize) break; // Stillstand verhindern
    }
    return false;
}

void AppendOggPacketPages(std::vector<uint8_t>& out, const ByteArrayPtr& packet, uint32_t serialNumber,
                          uint32_t& sequenceNumber, int64_t granulePos, uint8_t firstPageFlags)
{
    const uint8_t* data = static_cast<const uint8_t*>(packet.ptr());
    const size_t totalSize = packet.size();
    size_t pos = 0;
    bool firstPage = true;

    do {
        std::vector<uint8_t> segments;
        size_t pagePayloadSize = 0;

        while (segments.size() < 255) {
            const size_t remaining = totalSize - pos - pagePayloadSize;
            if (remaining >= 255) {
                segments.push_back(255);
                pagePayloadSize += 255;
            } else {
                segments.push_back((uint8_t)remaining);
                pagePayloadSize += remaining;
                break;
            }
        }

        const bool packetFinished = (pos + pagePayloadSize == totalSize) && (segments.empty() || segments.back() < 255);
        uint8_t flags = firstPage ? firstPageFlags : 0x01; // continuation flag fuer Folgepages desselben Pakets
        const int64_t pageGranule = packetFinished ? granulePos : -1;

        const size_t pageStart = out.size();
        const size_t headerSize = 27 + segments.size();
        out.resize(pageStart + headerSize + pagePayloadSize);

        char* hdr = reinterpret_cast<char*>(out.data() + pageStart);
        memcpy(hdr, "OggS", 4);
        Poke8(hdr + 4, 0); // stream_structure_version
        Poke8(hdr + 5, flags);
        Poke64(hdr + 6, (uint64_t)pageGranule);
        Poke32(hdr + 14, serialNumber);
        Poke32(hdr + 18, sequenceNumber++);
        Poke32(hdr + 22, 0); // CRC-Platzhalter
        Poke8(hdr + 26, (uint8_t)segments.size());
        if (!segments.empty()) {
            memcpy(hdr + 27, segments.data(), segments.size());
        }
        if (pagePayloadSize > 0 && data != nullptr) {
            memcpy(hdr + headerSize, data + pos, pagePayloadSize);
        }

        const uint32_t crc = OggCrc32(hdr, headerSize + pagePayloadSize);
        Poke32(hdr + 22, crc);

        pos += pagePayloadSize;
        firstPage = false;

        if (packetFinished) break;
    } while (true);
}

void BuildFlacPictureBlock(const ID3Tag::FlacPicture& pic, ByteArray& out)
{
    const size_t imgLen = pic.data.size();
    const size_t totalSize = 4 + 4 + pic.mime.size() + 4 + pic.description.size() + 16 + 4 + imgLen;
    char* buf = static_cast<char*>(out.malloc(totalSize));
    size_t pos = 0;
    PokeN32(buf + pos, pic.type);
    pos += 4;
    PokeN32(buf + pos, (uint32_t)pic.mime.size());
    pos += 4;
    if (!pic.mime.isEmpty()) {
        memcpy(buf + pos, pic.mime.c_str(), pic.mime.size());
        pos += pic.mime.size();
    }
    PokeN32(buf + pos, (uint32_t)pic.description.size());
    pos += 4;
    if (!pic.description.isEmpty()) {
        memcpy(buf + pos, pic.description.c_str(), pic.description.size());
        pos += pic.description.size();
    }
    memset(buf + pos, 0, 16); // width, height, color depth, indexed colors
    pos += 16;
    PokeN32(buf + pos, (uint32_t)imgLen);
    pos += 4;
    if (imgLen > 0) {
        memcpy(buf + pos, pic.data.ptr(), imgLen);
    }
}

} // namespace pplib
