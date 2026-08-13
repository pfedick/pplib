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
#include <config_pplib.h>
#include <pplib/exceptions.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#endif

#include "pplib.h"
#include "pplib-inet.h"
// #include "socket_pplib.h"

// #define DEBUGOUT

namespace pplib
{

/*!\class CSocketMessage
 * \ingroup PPLGroupInternet
 */

SocketMessage::SocketMessage()
{
    payload_type = 0;
    commandId = 0;
    ClientSupportsCompression = false;
    Version = 1;
    UseCompression = true;
    Id = 0;
    SupportMsgChannel = false;
}

SocketMessage::SocketMessage(const SocketMessage& other)
    : SocketMessage()
{
    copy(other);
}

SocketMessage::~SocketMessage()
{
    clear();
}

void SocketMessage::clear()
{
    payload.clear();
}

void SocketMessage::copy(const SocketMessage& other)
{
    clear();
    commandId = other.commandId;
    Id = other.Id;
    Version = other.Version;
    UseCompression = other.UseCompression;
    payload = other.payload;
    payload_type = other.payload_type;
}

void SocketMessage::enableCompression(bool flag)
{
    UseCompression = flag;
}

void SocketMessage::enableMsgChannel(bool flag)
{
    SupportMsgChannel = flag;
}

bool SocketMessage::isCompressionSupported() const
{
    return ClientSupportsCompression;
}

bool SocketMessage::isMsgChannelSupported() const
{
    return SupportMsgChannel;
}

void SocketMessage::setCommandId(int id)
{
    commandId = id;
}

void SocketMessage::setId(int id)
{
    Id = id;
}

void SocketMessage::setPayload(const String& msg)
{
    clear();
    payload_type = Variant::TYPE_STRING;
    payload.copy(msg.c_str(), msg.size());
}

void SocketMessage::setPayload(const AssocArray& msg)
{
    clear();
    payload_type = Variant::TYPE_ASSOCARRAY;
    msg.exportBinary(payload);
}

void SocketMessage::setPayload(const ByteArrayPtr& msg)
{
    clear();
    payload_type = Variant::TYPE_BYTEARRAY;
    payload.copy(msg.ptr(), msg.size());
}

void SocketMessage::getPayload(String& msg) const
{
    if (payload.isEmpty()) {
        throw NoDataAvailableException();
    }
    if (payload_type == 1 || payload_type == Variant::TYPE_STRING) {
        msg.set((const char*)payload, payload.size());
        return;
    }
    throw DataInOtherFormatException();
}

void SocketMessage::getPayload(AssocArray& msg) const
{
    if (payload.isEmpty()) {
        throw NoDataAvailableException();
    }
    if (payload_type != Variant::TYPE_ASSOCARRAY) {
        throw DataInOtherFormatException();
    }
    msg.importBinary(payload);
}

void SocketMessage::getPayload(ByteArray& msg) const
{
    if (payload.isEmpty()) {
        throw NoDataAvailableException();
    }
    if (payload_type != Variant::TYPE_BYTEARRAY) {
        throw DataInOtherFormatException();
    }
    msg.copy(payload);
}

int SocketMessage::getId() const
{
    return Id;
}

int SocketMessage::getCommandId() const
{
    return commandId;
}

int SocketMessage::getPayloadType()
{
    if (payload.isEmpty()) {
        throw NoDataAvailableException();
    }
    return payload_type;
}

void SocketMessage::compilePacketHeader(
    char* buffer, size_t* buffer_size, const void* payload, size_t payload_size, bool is_compressed) const
{
    if (*buffer_size < 24) throw BufferTooSmallException();
    memset(buffer, 0, 24);
    int flags = 0;
    if (UseCompression) flags |= 2;    // Bit 1: Client supports ZLib
    if (SupportMsgChannel) flags |= 4; // Bit 2: Client supports MsgChannel
    if (is_compressed) flags |= 1;
    PokeN8(buffer, 'V');               // Byte 0:  "V"							(1 Byte)
    PokeN8(buffer + 1, 2);             // Byte 1:  Version=2					(1 Byte)
    PokeN16(buffer + 2, commandId);    // Byte 2:  CommandId					(2 Byte)
    PokeN32(buffer + 4, Id);           // Byte 4:  Id							(4 Byte)
    PokeN32(buffer + 8, payload_size); // Byte 8:  Bytes Nutzdaten				(4 Byte)
    PokeN8(buffer + 12, flags);        // Byte 12: Flags						(1 Byte)
    //  											        Bit 0: Zlib-Kompression
    //														Bit 1: Client supports ZLib
    //														Bit 2: Client supports MsgChannel
    PokeN8(buffer + 13, payload_type);      // Byte 13: Datatype, PPL_ARRAY, usw.	(1 Byte)
    PokeN16(buffer + 14, rand(177, 65534)); // Byte 14: Zufallszahl					(2 Byte)
    uint32_t crc_data = 0;
    if (payload_size) crc_data = Crc32(payload, payload_size);
    PokeN32(buffer + 16, crc_data);          // Byte 16: CRC-Summe ueber die Daten	(4 Byte)
    PokeN32(buffer + 20, Crc32(buffer, 20)); // Byte 20: CRC-Summe ueber den Header	(4 Byte)
    *buffer_size = 24;
}

size_t SocketMessage::readFromPacketHeader(const char* msgbuffer, int& flags)
{
    commandId = PeekN16(msgbuffer + 2);
    flags = PeekN8(msgbuffer + 12);
    payload_type = PeekN8(msgbuffer + 13);
    Id = PeekN32(msgbuffer + 4);
    size_t payload_size = PeekN32(msgbuffer + 8);
    if (Version == 1) {
        if (PeekN32(msgbuffer + 16) != Crc32(msgbuffer, 16)) throw InvalidPacketException("CRC checksum");
    } else if (PeekN8(msgbuffer) == 'V' && PeekN8(msgbuffer + 1) == 2) {
        // int data_crc=PeekN32(msgbuffer+16);
        if (PeekN32(msgbuffer + 20) != Crc32(msgbuffer, 20)) throw InvalidPacketException("CRC checksum");
    }
    if (flags & 2)
        ClientSupportsCompression = true;
    else
        ClientSupportsCompression = false;

    if (flags & 4)
        SupportMsgChannel = true;
    else
        SupportMsgChannel = false;
    return payload_size;
}

// #define DEBUG_LOG 1

size_t TCPSocket::write(const SocketMessage& msg)
/*!\brief Nachricht verschicken
 *
 * \desc
 *
 * \copydoc PPLSocketMessage
 */
{
    Compression comp(Compression::Algo_ZLIB, Compression::Level_High);
    ByteArray compressed;
    comp.usePrefix(Compression::Prefix_V1);
    const void* msg_payload = msg.payload.ptr();
    size_t msg_size = msg.payload.size();
    bool is_compressed = false;
    if (msg_size > 64 && msg.ClientSupportsCompression == true && msg.UseCompression == true) {
        try {
            comp.compress(compressed, msg_payload, msg_size);
            if (compressed.size() < msg_size) {
                is_compressed = true;
                msg_payload = compressed.ptr();
                msg_size = compressed.size();
            }
        }
        catch (...) {
        }
    }
    if (msg_size > 0xffffffff) throw SocketMessage::PayloadTooBigException();
    char header[24];
    size_t headersize = 24;
    msg.compilePacketHeader(header, &headersize, msg_payload, msg_size, is_compressed);
    size_t bytes_send = 0;
#ifdef DEBUG_LOG
    printf("Send Header:\n");
    pplib::HexDump(header, headersize);
#endif
    bytes_send += write(header, headersize);
    if (msg_size) {
#ifdef DEBUG_LOG
        printf("Send Payload:\n");
        pplib::HexDump(msg_payload, msg_size);
#endif
        bytes_send += write(msg_payload, msg_size);
    }
    return bytes_send;
}

bool TCPSocket::waitForMessage(SocketMessage& msg, int timeout_seconds, Thread* watch_thread)
/*!\brief Auf Nachricht warten
 *
 * \desc
 *
 * \copydoc PPLSocketMessage
 */
{
    Compression comp(Compression::Algo_ZLIB, Compression::Level_High);
    ByteArray uncompressed;
    comp.usePrefix(Compression::Prefix_V1);
    uint64_t tt = GetTime() + timeout_seconds;
    char msgbuffer[28];
    void* buffer = NULL;
    int flags;
    while (timeout_seconds == 0 || GetTime() <= tt) {
        if (watch_thread) {
            if (watch_thread->threadShouldStop()) {
                return false;
            }
        }
        memset(msgbuffer, 0, 24);
        if (!waitForIncomingData(0, 100000)) continue;

        // Datenpaket vorhanden
        this->readLoop(msgbuffer, 20, timeout_seconds, watch_thread);
        msg.Version = PeekN8(msgbuffer + 1);
#ifdef DEBUG_LOG
        printf("received Header:\n");
        HexDump(msgbuffer, 20);
#endif
        if (msgbuffer[0] == 'V' && msg.Version == 2) {
            this->readLoop(msgbuffer + 20, 4, timeout_seconds, watch_thread);
#ifdef DEBUG_LOG
            printf("received additional Header:\n");
            HexDump(msgbuffer + 20, 4);
#endif
        } else if (msgbuffer[0] != 'V' || msg.Version != 1) {
            throw SocketMessage::InvalidProtocolVersion();
        }
        msg.clear();
        size_t payload_size = msg.readFromPacketHeader(msgbuffer, flags);
        if (payload_size) {
            ByteArray buffer;
            buffer.malloc(payload_size);
            this->readLoop((void*)buffer.ptr(), payload_size, timeout_seconds, watch_thread);
#ifdef DEBUG_LOG
            printf("received Payload:\n");
            HexDump(buffer, msg.payload_size);
#endif

            if (msg.Version > 1) { // CRC prüfen
                if (PeekN32(msgbuffer + 16) != Crc32(buffer.ptr(), payload_size)) {
                    throw SocketMessage::InvalidPacketException("CRC checksum of payload");
                }
            }
            if (flags & 1) {
                comp.uncompress(uncompressed, buffer.ptr(), payload_size);
                msg.payload.copy(uncompressed.ptr(), uncompressed.size());
            } else {
                msg.payload = buffer;
            }
        }
        return true;
    }
    return false;
}

} // namespace pplib
