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

#ifndef PPLIB_AUDIO_OGGVORBISHELPER_H
#define PPLIB_AUDIO_OGGVORBISHELPER_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <pplib/core/fileobject.h>
#include <pplib/types/bytearray.h>
#include <pplib/audio/id3tag.h>

namespace pplib
{

struct OggPageHeader
{
    uint8_t version = 0;
    uint8_t headerType = 0;
    int64_t granulePos = -1;
    uint32_t serialNumber = 0;
    uint32_t sequenceNumber = 0;
    uint32_t crc = 0;
    uint8_t numSegments = 0;
    uint32_t headerSize = 0;
    uint32_t payloadSize = 0;
};

struct OggVorbisHeaders
{
    uint32_t serialNumber = 0;
    uint8_t channels = 0;
    uint32_t frequency = 0;
    int32_t bitrateMax = 0;
    int32_t bitrateNominal = 0;
    int32_t bitrateMin = 0;
    ByteArray idPacket;
    ByteArray commentPacket;
    ByteArray setupPacket;
    uint64_t audioPageStart = 0;
    uint32_t nextSequenceNumber = 0;
};

uint32_t OggCrc32Update(uint32_t crc, const void* data, size_t size);
uint32_t OggCrc32(const void* data, size_t size);

bool ReadOggPageHeader(FileObject& file, uint64_t offset, uint64_t fileSize, OggPageHeader& outHeader,
                       std::vector<uint8_t>* outSegments = nullptr);

bool ReadOggVorbisHeaders(FileObject& file, OggVorbisHeaders& out, bool needSetupPacket);

bool FindLastOggGranulePos(FileObject& file, uint32_t serialNumber, uint64_t minOffset, int64_t& outGranulePos,
                           uint64_t& outLastPageEnd);

void AppendOggPacketPages(std::vector<uint8_t>& out, const ByteArrayPtr& packet, uint32_t serialNumber,
                          uint32_t& sequenceNumber, int64_t granulePos, uint8_t firstPageFlags);

void BuildFlacPictureBlock(const ID3Tag::FlacPicture& pic, ByteArray& out);

} // namespace pplib

#endif // PPLIB_AUDIO_OGGVORBISHELPER_H
