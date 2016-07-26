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

#include "prolog.h"
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef _WIN32
    #include <winsock2.h>
#endif

#include "ppl7.h"
#include "ppl7-inet.h"
#include "socket.h"


//#define DEBUGOUT

namespace ppl7 {

/*!\class CSocketMessage
 * \ingroup PPLGroupInternet
 */

SocketMessage::SocketMessage()
{
	datatype=0;
	data=NULL;
	size=0;
	commandId=0;
	incoming_data=NULL;
	incoming_size=0;
	incoming_type=0;
	ClientSupportsCompression=false;
	Version=1;
	UseCompression=true;
	Id=0;
	SupportMsgChannel=false;
}

SocketMessage::SocketMessage(const SocketMessage &other)
{
	datatype=0;
	data=NULL;
	size=0;
	commandId=0;
	incoming_data=NULL;
	incoming_size=0;
	incoming_type=0;
	ClientSupportsCompression=false;
	Version=1;
	UseCompression=true;
	SupportMsgChannel=false;
	Id=0;
	copy(other);
}

SocketMessage::~SocketMessage()
{
	clear();
}

void SocketMessage::clear()
{
	data=NULL;
	datatype=0;
	size=0;
	if (incoming_data) free(incoming_data);
	incoming_data=NULL;
	incoming_size=0;
	incoming_type=0;
}

void SocketMessage::copy(const SocketMessage &other)
{
	clear();
	datatype=other.datatype;
	size=other.size;
	incoming_size=other.incoming_size;
	incoming_type=other.incoming_type;
	commandId=other.commandId;
	Id=other.Id;
	Version=other.Version;
	UseCompression=other.UseCompression;
	data=other.data;
	incoming_data=NULL;
	if (incoming_size) {
		incoming_data=malloc(incoming_size+1);
		if (!incoming_data) {
			incoming_size=0;
		} else {
			memcpy(incoming_data,other.incoming_data,incoming_size);
		}
	}
}

void SocketMessage::enableCompression(bool flag)
{
	UseCompression=flag;
}

void SocketMessage::enableMsgChannel(bool flag)
{
	SupportMsgChannel=flag;
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
	commandId=id;
}

void SocketMessage::setId(int id)
{
	Id=id;
}

void SocketMessage::setData(const String &msg)
{
	//printf ("SetData: msg->size()=%i, Ptr=>>%s<<\n",msg->Size(),msg->GetPtr());
	datatype=Variant::TYPE_STRING;
	data=(void*)&msg;
}

void SocketMessage::setData(const AssocArray &msg)
{
	datatype=Variant::TYPE_ASSOCARRAY;
	data=(void*)&msg;
}

#ifdef TODO

void CSocketMessage::Dump()
{
	CString Buffer;
	Dump(Buffer);
	Buffer.Print(true);
}

void CSocketMessage::Dump(CLog *Log, int facility, int level)
{
	CString Buffer;
	Dump(Buffer);
	Log->Print(facility,level,"ppl6::CSocketMessage","Dump",__FILE__,__LINE__,(const char*)Buffer);
}

void CSocketMessage::Dump(CString &buffer)
{
	const char *t="unknown";
	switch (datatype) {
		case PPL_CHAR: t="PPL_CHAR";
			break;
		case PPL_STRING: t="PPL_STRING";
			break;
		case PPL_ASSOCARRAY: t="PPL_ASSOCARRAY";
			break;
	};
	buffer.Sprintf ("CSocketMessage:\n");
	buffer.Concatf ("   CommandId:     %i\n",commandId);
	buffer.Concatf ("   Id:            %i\n",Id);
	buffer.Concatf ("   Datatype Send: %s\n",t);
	switch (datatype) {
		case PPL_ASSOCARRAY:
			((CAssocArray*)data)->List();
			break;
		case PPL_STRING:
			((CString*)data)->Print(true);
			break;
		case PPL_CHAR:
			buffer.Concatf("%s\n",(char*)data);
			break;
	};
	t="unknown";
	switch (incoming_type) {
		case PPL_CHAR: t="PPL_CHAR";
			break;
		case PPL_STRING: t="PPL_STRING";
			break;
		case PPL_ASSOCARRAY: t="PPL_ASSOCARRAY";
			break;
	};
	CAssocArray a;
	buffer.Concatf ("   Datatype Read: %s\n",t);
	buffer.Concatf ("   Size:          %i Bytes\n",incoming_size);
	switch (incoming_type) {
		case PPL_ASSOCARRAY:
			if (GetData(a)) {
				a.List();
			} else {
				PrintError();
			}
			break;
		case PPL_STRING:
		case PPL_CHAR:
			buffer.Concatf("%s\n",(char*)incoming_data);
			break;
	};
}

const char *CSocketMessage::GetData()
{
	if (!incoming_data) {
		SetError(338);
		return NULL;
	}
	if (incoming_type==PPL_CHAR || incoming_type==PPL_STRING) {
		return (const char*)incoming_data;
	}
	SetError(337);
	return NULL;
}

#endif


void SocketMessage::getData(String &str)
{
	if (!incoming_data) {
		throw NoDataAvailableException();
	}
	if (incoming_type==1 || incoming_type==Variant::TYPE_STRING) {
		str.set((const char*)incoming_data);
		return;
	}
	throw DataInOtherFormatException();
}

void SocketMessage::getData(AssocArray &a)
{
	if (!incoming_data) {
		throw NoDataAvailableException();
	}
	if (incoming_type!=Variant::TYPE_ASSOCARRAY) {
		throw DataInOtherFormatException();
	}
	a.importBinary(incoming_data,incoming_size);
}

int SocketMessage::getId()
{
	return Id;
}

int SocketMessage::getCommandId()
{
	return commandId;
}

int SocketMessage::getType()
{
	if (!incoming_data) {
		throw NoDataAvailableException();
	}
	return incoming_type;
}

void SocketMessage::setVersion(int version)
{
	if (version==1 || version==2 ) {
		Version=1;
		return;
	}
	throw InvalidProtocolVersion("SocketMessage::setVersion = %d", version);
}




size_t TCPSocket::write(const SocketMessage &msg)
/*!\brief Nachricht verschicken
 *
 * \desc
 *
 * \copydoc PPLSocketMessage
 */
{
	Compression comp(Compression::Algo_ZLIB,Compression::Level_High);
	ByteArray compressed;
	char header[24];
	comp.usePrefix(Compression::Prefix_V1);
	bzero(header,24);
	int flags=0;
	if (msg.UseCompression) flags|=2;		// Bit 1: Client supports ZLib
	if (msg.SupportMsgChannel) flags|=4;	// Bit 2: Client supports MsgChannel
	bool freedata=false;
	void *data=NULL;
	const void *send=NULL;
	size_t size=0;
	ppluint32 crc=0;
	int rnd=rand(177,65534);
	size_t msg_size=0;
	switch (msg.datatype) {
		case Variant::TYPE_ASSOCARRAY:
			msg_size=((AssocArray*)(msg.data))->size();
			data=malloc(msg_size);
			freedata=true;
			try {
				((AssocArray*)(msg.data))->exportBinary(data,msg_size,&size);
			} catch (...) {
				free(data);
				throw;
			}
			msg_size=size;
			break;
		case Variant::TYPE_STRING:
			data=(void*)((String*)(msg.data))->getPtr();
			msg_size=((String*)(msg.data))->size();
			break;
		case 1:
			data=msg.data;
			msg_size=strlen((char*)data)+1;
			break;
	};
	if (msg_size>64 && msg.ClientSupportsCompression==true && msg.UseCompression==true) {
		try {
			comp.compress(compressed,data,msg_size);
			send=compressed.ptr();
			size=compressed.size();
			if (size>msg_size) {
				// Nachricht ist komprimiert größer als unkomprimiert, wir senden daher unkomprimiert
				size=msg_size;
				send=data;
			} else {
				flags|=1;
			}
		} catch (...) {
			size=msg_size;
			send=data;
		}
	} else {
		size=msg_size;
		send=data;
	}
	int headersize=0;
	//printf ("MSG Size: %i Bytes\n",size);
	if (msg.Version==2) {
		PokeN8(header,'V');							// Byte 0:  "V"							(1 Byte)
		PokeN8(header+1,2);							// Byte 1:  Version=2					(1 Byte)
		PokeN16(header+2,msg.commandId);			// Byte 2:  CommandId					(2 Byte)
		PokeN32(header+4,msg.Id);					// Byte 4:  Id							(4 Byte)
		PokeN32(header+8,size);						// Byte 8:  Bytes Nutzdaten				(4 Byte)
		PokeN8(header+12,flags);					// Byte 12: Flags						(1 Byte)
		//  											        Bit 0: Zlib-Kompression
		//														Bit 1: Client supports ZLib
		PokeN8(header+13,msg.datatype);			// Byte 13: Datatype, PPL_ARRAY, usw.	(1 Byte)
		PokeN16(header+14,rnd);						// Byte 14: Zufallszahl					(2 Byte)
		ppluint32 crc_data=0;
		if (size) crc_data=Crc32(send,size);
		PokeN32(header+16,crc_data);				// Byte 16: CRC-Summe ueber die Daten	(4 Byte)
		crc=Crc32(header,20);
		PokeN32(header+20,crc);						// Byte 20: CRC-Summe ueber den Header	(4 Byte)
		headersize=24;
	} else {
		PokeN8(header,'V');							// Byte 0:  "V"							(1 Byte)
		PokeN8(header+1,1);							// Byte 1:  Version=1					(1 Byte)
		PokeN16(header+2,msg.commandId);			// Byte 2:  CommandId					(2 Byte)
		PokeN32(header+4,msg.Id);					// Byte 4:  Id							(4 Byte)
		PokeN32(header+8,size);						// Byte 8:  Bytes Nutzdaten				(4 Byte)
		PokeN8(header+12,flags);					// Byte 12: Flags						(1 Byte)
		// 														Bit 0: Zlib-Kompression
		//														Bit 1: Client supports ZLib
		PokeN8(header+13,msg.datatype);			// Byte 13: Datatype, PPL_ARRAY, usw.	(1 Byte)
		PokeN16(header+14,rnd);						// Byte 14: Zufallszahl					(2 Byte)
		crc=Crc32(header,16);
		PokeN32(header+16,crc);						// Byte 16: CRC-Summe ueber den Header	(4 Byte)
		headersize=20;
	}
	size_t bytes_send=0;
	try {
		bytes_send+=write(header,headersize);
		if (size) {
			bytes_send+=write(send,size);
		}
	} catch (...) {
		if (freedata) free(data);
		throw;
	}
	if (freedata) free(data);
	return bytes_send;
}

#ifdef TODO

int TCPSocket::waitForMessage(SocketMessage &msg, int timeout_seconds)
/*!\brief Auf Nachricht warten
 *
 * \desc
 *
 * \copydoc PPLSocketMessage
 */
{
	CCompression comp(CCompression::Algo_ZLIB,CCompression::Level_High);
	CBinary uncompressed;
	comp.UsePrefix(CCompression::Prefix_V1);
	ppluint64 tt=GetTime()+timeout;
	char msgbuffer[28];
	void *buffer=NULL;
	void *newbuffer=NULL;
	int flags,type,size,validheader;
	ppluint32 crc=0, data_crc=0;
	int version=0;
	CString hex;
	while (timeout==0 || GetTime()<=tt) {
		if (thread) {
			if (thread->ThreadShouldStop()) {
				SetError(336);
				return 0;
			}
		}
		bzero(msgbuffer,24);
		if (!WaitForIncomingData(0,100000)) {
			if (GetErrorCode()==174) continue;
			return 0;
		}
		if (!this->Read(msgbuffer,20)) {
			return 0;
		}
		msg.commandId=PeekN16(msgbuffer+2);
		flags=PeekN8(msgbuffer+12);
		type=PeekN8(msgbuffer+13);
		msg.Id=PeekN32(msgbuffer+4);
		size=PeekN32(msgbuffer+8);
		validheader=0;
		if (PeekN8(msgbuffer)=='V' && PeekN8(msgbuffer+1)==1) {
			data_crc=0;
			crc=crc32(msgbuffer,16);
			if (crc==PeekN32(msgbuffer+16)) validheader=1;
			else {
				hex.Setf("Version Soll: V1, Ist: %c%i, CRC Soll: %x, Ist: %x\n",
					PeekN8(msgbuffer),PeekN8(msgbuffer+1),
					crc,
					PeekN32(msgbuffer+16));
				hex.HexDump(msgbuffer,20);
			}
			version=1;
		} else if (PeekN8(msgbuffer)=='V' && PeekN8(msgbuffer+1)==2) {
			if (!this->Read(msgbuffer+20,4)) return 0;
			data_crc=PeekN32(msgbuffer+16);
			crc=crc32(msgbuffer,20);
			if (crc==PeekN32(msgbuffer+20)) validheader=1;
			else {
				hex.Setf("Version Soll: V2, Ist: %c%i, CRC Soll: %x, Ist: %x\n",
					PeekN8(msgbuffer),PeekN8(msgbuffer+1),
					crc,
					PeekN32(msgbuffer+20));
				hex.HexDump(msgbuffer,24);
			}
			version=2;
		}
		if (!validheader) {
			SetError(346,(const char*)hex);
			return 0;
		}
		if (flags&2) {
			msg.ClientSupportsCompression=true;
		}
		else msg.ClientSupportsCompression=false;
		if (flags&4) msg.SupportMsgChannel=true;
		else msg.SupportMsgChannel=false;

		//printf ("crc ok\n");
		msg.Clear();
		msg.Version=version;
		if (size) {
			buffer=malloc(size+1);
			if (!buffer) {
				SetError(2);
				return 0;
			}
			if (!this->Read(buffer,size)) {
				free(buffer);
				return 0;
			}
			if (version>1) {	// CRC prüfen
				crc=crc32(buffer,size);
				if (crc!=data_crc) {
					free(buffer);
					SetError(411,"Soll: %x, Ist: %x", data_crc, crc);
					return 0;
				}
			}
			//printf ("Read ok\n");
			if (flags&1) {
				//ppl6::HexDump(buffer,64);
				if (!comp.Uncompress(uncompressed,buffer,size,false)) {
					free(buffer);
					return 0;
				}
				size=uncompressed.Size();
				newbuffer=realloc(buffer,size+1);
				if (!newbuffer) {
					free(buffer);
					SetError(2);
					return 0;
				}
				buffer=newbuffer;
				memcpy(buffer,uncompressed.GetPtr(),size);
			}
			((char*)buffer)[size]=0;
			msg.incoming_data=buffer;
			msg.incoming_type=type;
			msg.incoming_size=size;
		}
		return 1;
	}
	SetError(174);
	return 0;
}


#endif // TODO
}


