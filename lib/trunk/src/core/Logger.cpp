/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.5 $
 * $Date: 2010/03/30 09:24:02 $
 * $Id: Logger.cpp,v 1.5 2010/03/30 09:24:02 pafe Exp $
 *
 *******************************************************************************
 * Copyright (c) 2012, Patrick Fedick <patrick@pfp.de>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ppl7.h"


namespace ppl7 {

typedef struct tagLOGHANDLER
{
	LogHandler *handler;
	struct tagLOGHANDLER *next;
	struct tagLOGHANDLER *previous;
} LOGHANDLER;


static const char *facilitylist[] = {
	"NONE   ",						// 0
	"EMERG  ",						// 1
	"ALERT  ",						// 2
	"CRIT   ",						// 3
	"ERROR  ",						// 4
	"WARNING",						// 5
	"NOTICE ",						// 6
	"INFO   ",						// 7
	"DEBUG  ",						// 8
	""
};



Logger::Logger()
{
	firsthandler=lasthandler=NULL;
	logconsole=false;
	logThreadId=true;
	console_enabled=false;
	console_facility=Logger::DEBUG;
	console_level=1;
	for (int i=0;i<NUMFACILITIES;i++) {
		debuglevel[i]=0;
	}
	FilterModule=NULL;
	FilterFile=NULL;
	rotate_mechanism=0;
	maxsize=1024*1024*1024;
	generations=1;
	inrotate=false;
	useSyslog=false;
}

Logger::~Logger()
{
	terminate();
	if (FilterModule) delete FilterModule;
	if (FilterFile) delete FilterFile;
}

void Logger::terminate()
{
	print(Logger::INFO,0,"ppl7::Logger","terminate",__FILE__,__LINE__,"=== Logfile-Class terminated ===============================");
	for (int i=0;i<NUMFACILITIES;i++) {
		logff[i].close();
	}
	mutex.lock();
	LOGHANDLER *c,*h=(LOGHANDLER *)firsthandler;
	while (h) {
		c=h;
		h=h->next;
		free(c);
	}
	firsthandler=lasthandler=NULL;
	logconsole=false;
	useSyslog=false;
	logThreadId=true;
	console_enabled=false;
	console_facility=Logger::DEBUG;
	console_level=1;
	for (int i=0;i<NUMFACILITIES;i++) {
		debuglevel[i]=0;
	}
	mutex.unlock();
}



void Logger::enableConsole(bool flag, int facility, int level)
{
	mutex.lock();
	console_enabled=flag;
	console_facility=facility;
	console_level=level;
	mutex.unlock();
}

void Logger::enableSyslog(bool flag)
{
	if (useSyslog==true && flag==false) {
		print(Logger::INFO,0,"ppl7::Logger","terminate",__FILE__,__LINE__,"=== Disable Syslog ===============================");
	} else if (useSyslog==false && flag==true) {
		print(Logger::INFO,0,"ppl7::Logger","terminate",__FILE__,__LINE__,"=== Enable Syslog ===============================");
	}
	useSyslog=flag;
}


void Logger::setLogfile(int facility, const char *filename)
{
	if (facility<1 || facility>=NUMFACILITIES) return;
	mutex.lock();
	try {
		logff[facility].close();
		if (filename) {
			logff[facility].open(filename,File::APPEND);
			print(facility,0,"ppl7::Logger","SetLogfile",__FILE__,__LINE__,ToString("=== Logfile started for %s",facilitylist[facility]));
		}
	} catch (...) {
		mutex.unlock();
		throw;
	}
}

void Logger::setLogRotate(ppluint64 maxsize, int generations)
{
	if (generations>1 && maxsize>1024*1024) {
		mutex.lock();
		this->maxsize=maxsize;
		this->generations=generations;
		rotate_mechanism=1;
		mutex.unlock();
	}
}

void Logger::setLogLevel(int facility, int level)
{
	if (facility>=0 && facility<NUMFACILITIES) {
		mutex.lock();
		debuglevel[facility]=level;
		mutex.unlock();
		print(facility,0,"ppl7::Logger","SetLogLevel",__FILE__,__LINE__,ToString("=== Setting Debuglevel for %s, to: %u",facilitylist[facility],level));
	}
}

int Logger::getLogLevel(int facility)
{
	int ret=0;
	if (facility>=0 && facility<NUMFACILITIES) {
		mutex.lock();
		ret=debuglevel[facility];
		mutex.unlock();
	}
	return ret;
}


void Logger::print (const String &text)
{
	if (!shouldPrint(NULL,NULL,NULL,NULL,Logger::DEBUG,0)) return;
	mutex.lock();
	output(Logger::DEBUG,0,NULL,NULL,NULL,0,text,true);
	mutex.unlock();
}

void Logger::print (int level, const String &text)
{
	if (!shouldPrint(NULL,NULL,NULL,NULL,Logger::DEBUG,level)) return;
	mutex.lock();
	output(Logger::DEBUG,level,NULL,NULL,NULL,0,text,true);
	mutex.unlock();
}

void Logger::print (int facility, int level, const String &text)
{
	if (!shouldPrint(NULL,NULL,NULL,NULL,facility,level)) return;
	mutex.lock();
	output(facility,level,NULL,NULL,NULL,0,text,true);
	mutex.unlock();
}

void Logger::print (int facility, int level, const char *file, int line, const String &text)
{
	if (!shouldPrint(NULL,NULL,file,line,facility,level)) return;
	mutex.lock();
	output(facility,level,NULL,NULL,file,line,text,true);
	mutex.unlock();
}

void Logger::print (int facility, int level, const char *module, const char *function, const char *file, int line, const String &text)
{
	if (!shouldPrint(module,function,file,line,facility,level)) return;
	mutex.lock();
	output(facility,level,module,function,file,line,text,true);
	mutex.unlock();
}


void Logger::printArray (int facility, int level, const AssocArray &a, const String &text)
{
	if (!shouldPrint(NULL,NULL,NULL,NULL,facility,level)) return;
	mutex.lock();
	output(facility,level,NULL,NULL,NULL,0,text,true);
	outputArray(facility,level,NULL,NULL,NULL,0,a,NULL);
	mutex.unlock();
}

void Logger::printArray (int facility, int level, const char *module, const char *function, const char *file, int line, const AssocArray &a, const String &text)
{
	if (!shouldPrint(module,function,file,line,facility,level)) return;
	mutex.lock();
	output(facility,level,module,function,file,line,text,true);
	outputArray(facility,level,module,function,file,line,a,NULL);
	mutex.unlock();
}

void Logger::printArraySingleLine (int facility, int level, const char *module, const char *function, const char *file, int line, const AssocArray &a, const String &text)
{
	if (!shouldPrint(module,function,file,line,facility,level)) return;
	mutex.lock();
	String s;
	s=text;
	outputArray(facility,level,module,function,file,line,a,NULL,&s);
	s.replace("\n","; ");
	s.replace("    ","");
	output(facility,level,module,function,file,line,s,true);
	mutex.unlock();
}

#ifdef TODO

void Logger::outputArray(int facility, int level, const char *module, const char *function, const char *file, int line, const AssocArray &a, const char *prefix, String *Out)
{
	String key, pre, out;
	if (prefix) key.setf("%s/",prefix);

	if (!Out) Out=&out;
	AssocArray::Iterator walk;
	a.reset(walk);

	ARRAY_RESULT row;
	CString *string;
	CAssocArray *array;
	CBinary *binary;
	const char *k;
	void *pointer;
	while ((row=a->GetNext(walk))) {
		k=a->GetKey(row);
		if ((string=a->GetString(row))) {
			Out->Concatf("%s%s=%s\n",(const char*)key,k,string->GetPtr());
			//Output(facility,level,(char*)out,false);
		} else if ((pointer=a->GetPointer(row))) {
			Out->Concatf("%s%s=%llu\n",(const char*)key,k,((ppluint64)(ppliptr)pointer));
			//Output(facility,level,(char*)out,false);
		} else if ((array=a->GetArray(row))) {
			pre.Sprintf("%s%s",(const char*)key,k);
			OutputArray(facility,level,module,function,file,line,array,(const char*)pre,Out);
		} else if ((binary=a->GetBinary(row))) {
			Out->Concatf("%s%s=CBinary, %llu Bytes\n",(const char*)key,k,(ppluint64)binary->Size());
			//Output(facility,level,(char*)out,false);
		}
	}
	if (Out==&out) Output(facility,level,module,function,file,line,(const char*)out,false);
}

#endif

void Logger::hexDump (const void * address, int bytes)
{
	hexDump(Logger::DEBUG,1,address,bytes);
}

void Logger::hexDump (int facility, int level, const void * address, int bytes)
{
	if (!shouldPrint(NULL,NULL,NULL,NULL,facility,level)) return;
	if (address==NULL) return;
	mutex.lock();

	String line;
	String cleartext;

	char zeichen[2];
	zeichen[1]=0;
	//char buff[1024], tmp[10], cleartext[20];
	line.setf("HEXDUMP: %u Bytes starting at Address 0x%08llX (%llu):",
			bytes,(ppluint64)(ppliptr)address,(ppluint64)(ppliptr)address);
	output(facility,level,NULL,NULL,NULL,0,(const char*)line,true);

	char *_adresse=(char*)address;
	ppluint32 spalte=0;
	line.setf("0x%08llX: ",(ppluint64)(ppliptr)_adresse);
	cleartext.clear();
	for (int i=0;i<bytes;i++) {
		if (spalte>15) {
			line.append("                                                               ");
			line.chopRight(60);
			line.append(": ");
			line.append(cleartext);
			output(facility,level,NULL,NULL,NULL,0,(const char*)line,false);
			line.setf("0x%08llX: ",(ppluint64)(ppliptr)(_adresse+i));
			cleartext.clear();
			spalte=0;
		}
		line.appendf("%02X ",(ppluint8)_adresse[i]);
		zeichen[0]=(ppluint8)_adresse[i];
		if ((ppluint8)_adresse[i]>31)	cleartext.append(zeichen);
		else cleartext.append(".");
		spalte++;
	}

	if (spalte>0) {
		line.append("                                                               ");
		line.chopRight(60);
		line.append(": ");
		line.append(cleartext);
		output(facility,level,NULL,NULL,NULL,0,(const char*)line,false);
		output(facility,level,NULL,NULL,NULL,0,"",false);
	}
	mutex.unlock();
}

void Logger::printException(const Exception &e)
{

}

void Logger::printException(const char *file, int line, const Exception &e)
{

}

void Logger::printException(const char *file, int line, const char *module, const char *function, const Exception &e)
{

}

#ifdef TODO

void Logger::output(int facility, int level, const char *module, const char *function, const char *file, int line, const String &buffer, bool printdate)
{
	String bf;
	char d[20], z[20];
	if (printdate) {
		datum (d);
		zeit(z);
		double s=ppl7::GetMicrotime();
		s=(s-(ppluint64)s)*1000;
		if (logThreadId) bf.Sprintf("%s-%s.%03i [%7s %2i] [%6llu] ",d,z,(int)s,facilitylist[facility],level,GetThreadID());
		else bf.Sprintf("%s-%s [%7s %2i] ",d,z,facilitylist[facility],level);
		bf.Concat("[");
		if (file) bf.Concatf("%s:%i",file,line);
		bf.Concat("] {");
		if (module) {
			bf.Concatf("%s",module);
			if (function) bf.Concatf(": %s",function);
		}
		bf.Concat("}: ");
	} else {
		bf.Sprintf("     ");
	}
	CString bu=buffer;
	bu.Trim();
	bf+=bu;
	bf.Replace("\n","\n     ");
	bf.Concat("\n");

	if (level<=debuglevel[facility]) {
		logff[facility].Puts(bf);
		logff[facility].Flush();
		CheckRotate(facility);
	}
	if (facility!=LOG::DEBUG && level<=debuglevel[LOG::DEBUG] && (
		strcmp(logff[facility].GetFilename(),logff[LOG::DEBUG].GetFilename())!=0)) {
		logff[LOG::DEBUG].Puts(bf);
		logff[LOG::DEBUG].Flush();
		CheckRotate(LOG::DEBUG);
	}
	LOGHANDLER *h=(LOGHANDLER *)firsthandler;
	while (h) {
		h->handler->LogMessage(facility,level,(const char*)bf);
		h=h->next;
	}
	PopError();
}

void Logger::addLogHandler(LoggerHandler *handler)
{
	if (!handler) {
		SetError(194);
		return 0;
	}
	LOGHANDLER *h=(LOGHANDLER *)malloc(sizeof(LOGHANDLER));
	if (!h) {
		SetError(2);
		return 0;
	}
	h->handler=handler;
	h->previous=NULL;
	h->next=NULL;
	mutex.Lock();
	if (!lasthandler) {
		firsthandler=lasthandler=h;
		mutex.Unlock();
		return 1;
	}
	LOGHANDLER *last=(LOGHANDLER *)lasthandler;
	last->next=h;
	h->previous=last;
	lasthandler=h;
	mutex.Unlock();
	return 1;
}

int Logger::DeleteLogHandler(LoggerHandler *handler)
{
	if (!handler) {
		SetError(194);
		return 0;
	}
	mutex.Lock();
	LOGHANDLER *h=(LOGHANDLER *)firsthandler;
	if (!h) {
		mutex.Unlock();
		return 1;
	}
	while (h) {
		if (h->handler==handler) {
			if (h->previous) h->previous->next=h->next;
			if (h->next) h->next->previous=h->previous;
			if (h==firsthandler) firsthandler=h->next;
			if (h==lasthandler) lasthandler=h->previous;
			free(h);
			mutex.Unlock();
			return 1;
		}
		h=h->next;
	}
	mutex.Unlock();
	return 1;
}

int Logger::SetFilter(const char *module, const char *function, int level)
{
	if (!module) {
		SetError(194,"int Logger::SetFilter(==> const char *module <==, const char *file, int level)");
		return 0;
	}
	if (!FilterModule) FilterModule=new CAssocArray;
	if (!FilterModule) {
		SetError(2);
		return 0;
	}
	CString Name=module;
	if (function) Name.Concatf("::%s",function);
	FilterModule->Setf(Name,"%i",level);
	return 1;
}

int Logger::SetFilter(const char *file, int line, int level)
{
	if (!file) {
		SetError(194,"int Logger::SetFilter(==> const char *file <==, int line, int level)");
		return 0;
	}
	if (!FilterFile) FilterFile=new CAssocArray;
	if (!FilterFile) {
		SetError(2);
		return 0;
	}
	CString Name=file;
	Name.Concatf(":%i",line);
	FilterFile->Setf(Name,"%i",level);
	return 1;
}



void Logger::DeleteFilter(const char *module, const char *function)
{
	if (!FilterModule) return;
	if (!module) return;
	CString Name=module;
	if (function) Name.Concatf("::%s",function);
	FilterModule->Delete(Name);
}

void Logger::DeleteFilter(const char *file, int line)
{
	if (!FilterFile) return;
	if (!file) return;
	CString Name=file;
	Name.Concatf(":%i",line);
	FilterFile->Delete(Name);
}


bool Logger::shouldPrint(const char *module, const char *function, const char *file, int line, int facility, int level)
{
	if (facility<1 || facility>=NUMFACILITIES) return false;
	if (debuglevel[facility]<level) return false;				// Wenn der Debuglevel kleiner ist, brauchen wir nicht weiter machen
	bool ret=true;

	ppl7::PushError();
	if (IsFiltered(module,function,file,line,level)) ret=false;
	ppl7::PopError();
	return ret;
}

int Logger::IsFiltered(const char *module, const char *function, const char *file, int line, int level)
{
	CString Name;
	const char *tmp;
	int l;
	if (FilterModule) {
		if (module) {
			Name=module;
			tmp=FilterModule->Get(Name);
			if (tmp) {
				l=ppl7::atoi(tmp);
				if (level>=l) return 1;
			}
			if (function) {
				Name.Concatf("::%s",function);
				tmp=FilterModule->Get(Name);
				if (tmp) {
					l=ppl7::atoi(tmp);
					if (level>=l) return 1;
				}
			}
		}
	}
	if (FilterFile) {
		if (file) {
			Name.Setf("%s:0",file);
			tmp=FilterFile->Get(Name);
			if (tmp) {
				l=ppl7::atoi(tmp);
				if (level>=l) return 1;
			}
			Name.Setf("%s:%i",file,line);
			tmp=FilterFile->Get(Name);
			if (tmp) {
				l=ppl7::atoi(tmp);
				if (level>=l) return 1;
			}
		}
	}
	return 0;
}


void Logger::checkRotate(int facility)
{
	String f1,f2;
	if (inrotate) return;
	if (rotate_mechanism==1) {
		pplint64 size=logff[facility].size();
		if (size>0 && (ppluint64)size>maxsize) {
			inrotate=true;
			output(facility,0,"ppl7::Logger","CheckRotate",__FILE__,__LINE__,"Logfile Rotated");
			logff[facility].close();
			// Wir müssen die bisherigen Generationen umbenennen
			for (int i=generations;i>1;i--) {
				f1.Setf("%s.%i",logfilename[facility].GetPtr(),i-1);
				f2.Setf("%s.%i",logfilename[facility].GetPtr(),i);
				CFile::RenameFile(f1,f2);
			}
			f2.Setf("%s.1",logfilename[facility].GetPtr());
			CFile::RenameFile(logfilename[facility],f2);
			logff[facility].Open(logfilename[facility],"ab");
			f1.Setf("=== Logfile started for %s",facilitylist[facility]);
			Output(facility,0,"ppl7::Logger","SetLogfile",__FILE__,__LINE__,f1);
			inrotate=false;
		}
	}
}

#endif


} // EOF namespace ppl7

