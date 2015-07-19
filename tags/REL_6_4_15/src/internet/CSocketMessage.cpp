/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.7 $
 * $Date: 2010/03/26 11:51:32 $
 * $Id: CSocketMessage.cpp,v 1.7 2010/03/26 11:51:32 pafe Exp $
 *
 *******************************************************************************
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
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

#include "ppl6.h"
#include "socket.h"


//#define DEBUGOUT

namespace ppl6 {

/*!\class CSocketMessage
 * \ingroup PPLGroupInternet
 */


CSocketMessage::CSocketMessage()
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

CSocketMessage::CSocketMessage(const CSocketMessage &other)
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
	Copy(other);
}

CSocketMessage::~CSocketMessage()
{
	Clear();
}

void CSocketMessage::Copy(const CSocketMessage &other)
{
	Copy(&other);
}

void CSocketMessage::Copy(const CSocketMessage *other)
{
	Clear();
	if (!other) return;
	datatype=other->datatype;
	size=other->size;
	incoming_size=other->incoming_size;
	incoming_type=other->incoming_type;
	commandId=other->commandId;
	Id=other->Id;
	Version=other->Version;
	UseCompression=other->UseCompression;
	data=other->data;
	incoming_data=NULL;
	if (incoming_size) {
		incoming_data=malloc(incoming_size+1);
		if (!incoming_data) {
			incoming_size=0;
		} else {
			memcpy(incoming_data,other->incoming_data,incoming_size);
		}
	}
}



void CSocketMessage::EnableCompression()
{
	UseCompression=true;
}

void CSocketMessage::DisableCompression()
{
	UseCompression=false;
}

void CSocketMessage::Clear()
{
	data=NULL;
	datatype=0;
	size=0;
	if (incoming_data) free(incoming_data);
	incoming_data=NULL;
	incoming_size=0;
	incoming_type=0;
}

void CSocketMessage::SetCommandId(int id)
{
	commandId=id;
}

void CSocketMessage::SetId(int id)
{
	Id=id;
}

int CSocketMessage::SetData(const char *msg)
{
	if (!msg) {
		SetError(194,"char *msg");
		return 0;
	}
	datatype=PPL_ARRAY_CHAR;
	data=(void*)msg;
	return 1;
}

int CSocketMessage::SetData(const CString &msg)
{
	//printf ("SetData: msg->size()=%i, Ptr=>>%s<<\n",msg->Size(),msg->GetPtr());
	datatype=PPL_STRING;
	data=(void*)&msg;
	return 1;
}

int CSocketMessage::SetData(const CAssocArray &msg)
{
	datatype=PPL_ASSOCARRAY;
	data=(void*)&msg;
	return 1;
}

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

int CSocketMessage::GetData(CString &str)
{
	if (!incoming_data) {
		SetError(338);
		return 0;
	}
	if (incoming_type==PPL_CHAR || incoming_type==PPL_STRING) {
		str.Set((const char*)incoming_data);
		return 1;
	}
	SetError(337);
	return 0;
}

int CSocketMessage::GetData(CAssocArray &a)
{
	if (!incoming_data) {
		SetError(338);
		return 0;
	}
	if (incoming_type!=PPL_ASSOCARRAY) {
		SetError(337);
		return 0;
	}
	return a.ImportBinary(incoming_data,incoming_size);
}

int CSocketMessage::GetId()
{
	return Id;
}

int CSocketMessage::GetCommandId()
{
	return commandId;
}

int CSocketMessage::GetType()
{
	if (!incoming_data) {
		SetError(338);
		return 0;
	}
	return incoming_type;
}

int CSocketMessage::SetVersion(int version)
{
	if (version==1) {
		Version=1;
		return 1;
	}
	if (version==2) {
		Version=2;
		return 1;
	}
	SetError(412,"Behalte Version %i",Version);
	return 0;
}

void CSocketMessage::EnableMsgChannel()
{
	SupportMsgChannel=true;
}

void CSocketMessage::DisableMsgChannel()
{
	SupportMsgChannel=false;
}

bool CSocketMessage::isCompressionSupported() const
{
	return ClientSupportsCompression;
}

bool CSocketMessage::isMsgChannelSupported() const
{
	return SupportMsgChannel;
}


int CTCPSocket::Write(CSocketMessage &msg)
/*!\brief Nachricht verschicken
 *
 * \desc
 *
 * \copydoc PPLSocketMessage
 */
{
	if (log) log->Printf(LOG::DEBUG,4,"ppl6::CTCPSocket","Write",__FILE__,__LINE__,"Schreibe CSocketMessage");
	CCompression comp(CCompression::Algo_ZLIB,CCompression::Level_High);
	CBinary compressed;
	char header[24];
	comp.UsePrefix(CCompression::Prefix_V1);
	bzero(header,24);
	int flags=0;
	if (msg.UseCompression) flags|=2;		// Bit 1: Client supports ZLib
	if (msg.SupportMsgChannel) flags|=4;	// Bit 2: Client supports MsgChannel
	bool freedata=false;
	void *data=NULL;
	void *send=NULL;
	int size=0;
	ppluint32 crc=0;
	int rnd=rand(177,65534);
	msg.size=0;
	//msg->Dump();
	switch (msg.datatype) {
		case PPL_ASSOCARRAY:
			if (log) log->Printf(LOG::DEBUG,6,"ppl6::CTCPSocket","Write",__FILE__,__LINE__,"Exportiere CAssocArray, ermittle Speicherbedarf...");
			msg.size=((CAssocArray*)(msg.data))->Size();
			if (log) log->Printf(LOG::DEBUG,6,"ppl6::CTCPSocket","Write",__FILE__,__LINE__,"Exportiere CAssocArray mit %u Bytes", (ppluint32)msg.size);
			data=malloc(msg.size);
			freedata=true;
			if (!((CAssocArray*)(msg.data))->ExportBinary(data,msg.size,&size)) {
				PushError();
				free(data);
				PopError();
				return 0;
			}
			if (log) log->Printf(LOG::DEBUG,6,"ppl6::CTCPSocket","Write",__FILE__,__LINE__,"Exportiere CAssocArray fertig");
			msg.size=size;
			//printf ("Type: PPL_ASSOCARRAY, %i Bytes\n",msg.size);
			break;
		case PPL_STRING:
			data=(void*)((CString*)(msg.data))->GetPtr();
			msg.size=((CString*)(msg.data))->Size();
			//printf ("Type: PPL_STRING\n");
			break;
		case PPL_CHAR:
			//printf ("Type: PPL_CHAR\n");
			data=msg.data;
			msg.size=strlen((char*)data)+1;
			break;
		//default:
			//printf ("Type: unknown\n");
			//SetError(337);
			//return 0;
	};
	//printf ("MSG Size: %i Bytes\n",msg->size);
	//ppl6::HexDump(data,msg->size);
	if (msg.size>64 && msg.ClientSupportsCompression==true && msg.UseCompression==true) {
		//printf ("Verwende Komprimierung\n");
		if (log) log->Printf(LOG::DEBUG,4,"ppl6::CTCPSocket","Write",__FILE__,__LINE__,"Komprimiere Daten mit %u Bytes", (ppluint32)msg.size);
		if (!comp.Compress(compressed,data,msg.size,false)) {
			if (log) log->Printf(LOG::DEBUG,4,"ppl6::CTCPSocket","Write",__FILE__,__LINE__,"fehlgeschlagen, sende unkomprimiert");
			size=msg.size;
			send=data;
		} else {
			send=compressed.GetPtr();
			//msg.size=compressed.Size();
			size=compressed.Size();
			if (log) log->Printf(LOG::DEBUG,4,"ppl6::CTCPSocket","Write",__FILE__,__LINE__,"Komprimierte Groesse: %u Bytes", (ppluint32)size);
			if (size>msg.size) {
				// Nachricht ist komprimiert größer als unkomprimiert, wir senden daher unkomprimiert
				if (log) log->Printf(LOG::DEBUG,4,"ppl6::CTCPSocket","Write",__FILE__,__LINE__,"Nachricht ist komprimiert groesser, sende unkomprimiert");
				size=msg.size;
				send=data;
			} else {
				flags|=1;
			}
		}
	} else {
		size=msg.size;
		send=data;
		//printf ("Sende %i Bytes unkomprimiert\n",size);
		//HexDump(send,size);
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
		if (size) crc_data=crc32(send,size);
		PokeN32(header+16,crc_data);				// Byte 16: CRC-Summe ueber die Daten	(4 Byte)
		crc=crc32(header,20);
		PokeN32(header+20,crc);						// Byte 20: CRC-Summe ueber den Header	(4 Byte)
		headersize=24;
		//printf ("Version 2, CRC Soll: %x, Ist: %x\n",crc,PeekN32(header+20));
		//HexDump(header,headersize);
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
		crc=crc32(header,16);
		PokeN32(header+16,crc);						// Byte 16: CRC-Summe ueber den Header	(4 Byte)
		//Poke32(header+12,crc);	// Debug
		headersize=20;
		//printf ("Version 1, CRC Soll: %x, Ist: %x\n",crc,PeekN32(header+16));
	}
	/*
	printf ("version:    0x%08x (%i)\n",msg->Version,msg->Version);
	printf ("commandId:  0x%08x (%i)\n",msg->commandId,msg->commandId);
	printf ("Id:         0x%08x (%i)\n",msg->Id,msg->Id);
	printf ("size:       0x%08x (%i)\n",size,size);
	printf ("flags:      0x%08x (%i)\n",flags,flags);
	printf ("type:       0x%08x (%i)\n",msg->datatype,msg->datatype);
	printf ("rand:       0x%08x (%i)\n",rnd,rnd);
	printf ("crc:        0x%08x (%i)\n",crc,crc);
	HexDump(header,headersize);
	*/

	if (!Write(header,headersize)) {
		if (freedata) free(data);
		return 0;
	}
	if (size) {
		//printf ("Sende Daten...\n");
		//ppl6::HexDump(send,64);
		if (!Write(send,size)) {
			if (freedata) free(data);
			return 0;
		}
	}
	if (freedata) free(data);
	return 1;
}

int CTCPSocket::WaitForMessage(CSocketMessage &msg, int timeout)
/*!\brief Nachricht verschicken
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
#ifdef DEBUGOUT
	printf ("%010.3f CTCPSocket::WaitForMessage: Start\n",ppl6::GetMicrotime());
#endif
	while (timeout==0 || GetTime()<=tt) {
#ifdef DEBUGOUT
		printf ("%010.3f CTCPSocket::WaitForMessage: Prüfe Thread\n",ppl6::GetMicrotime());
#endif
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
#ifdef DEBUGOUT
		printf ("%010.3f CTCPSocket::WaitForMessage: Lese 20 Bytes\n",ppl6::GetMicrotime());
#endif
		if (!this->Read(msgbuffer,20)) {
#ifdef DEBUGOUT
			printf ("%010.3f CTCPSocket::WaitForMessage: Error, Abbruch\n",ppl6::GetMicrotime());
#endif
			return 0;
		}
#ifdef DEBUGOUT
		printf ("%010.3f CTCPSocket::WaitForMessage: 20 Bytes erhalten\n",ppl6::GetMicrotime());
#endif
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
#ifdef DEBUGOUT
			printf ("%010.3f CTCPSocket::WaitForMessage: Ungültiger Header -> Abbruch\n",ppl6::GetMicrotime());
#endif
			SetError(346,(const char*)hex);
			return 0;
		}
#ifdef DEBUGOUT
		printf ("%010.3f CTCPSocket::WaitForMessage: Ok, Version: %i\n",ppl6::GetMicrotime(),version);
#endif
		if (flags&2) {
#ifdef DEBUGOUT
			printf ("%010.3f CTCPSocket::WaitForMessage: Gegenstelle unterstützt komprimierung\n",ppl6::GetMicrotime());
#endif
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
#ifdef DEBUGOUT
			printf ("%010.3f CTCPSocket::WaitForMessage: Lese %u Bytes Daten\n",ppl6::GetMicrotime(),size);
#endif
			if (!this->Read(buffer,size)) {
#ifdef DEBUGOUT
				printf ("%010.3f CTCPSocket::WaitForMessage: Error\n",ppl6::GetMicrotime());
#endif
				free(buffer);
				return 0;
			}
#ifdef DEBUGOUT
			printf ("%010.3f CTCPSocket::WaitForMessage: Ok\n",ppl6::GetMicrotime());
#endif
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
#ifdef DEBUGOUT
				printf ("%010.3f CTCPSocket::WaitForMessage: Dekomprimiere Nachricht\n",ppl6::GetMicrotime());
#endif
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
#ifdef DEBUGOUT
				printf ("%010.3f CTCPSocket::WaitForMessage: Done\n",ppl6::GetMicrotime());
#endif
			}
			((char*)buffer)[size]=0;
			msg.incoming_data=buffer;
			msg.incoming_type=type;
			msg.incoming_size=size;
		}
#ifdef DEBUGOUT
		printf ("%010.3f CTCPSocket::WaitForMessage: Start\n",ppl6::GetMicrotime());
#endif
		return 1;
	}
	SetError(174);
	return 0;
}


}


