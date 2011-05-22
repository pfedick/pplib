/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
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

#ifdef HAVE_PTHREADS
	#include <pthread.h>
#endif


#include "ppl7.h"
#include "threads.h"

namespace ppl7 {

static ppluint64 global_thread_id=0;
static Mutex GlobalThreadMutex;



#ifdef _WIN32
Mutex Win32ThreadMutex;
DWORD Win32ThreadTLS=TLS_OUT_OF_INDEXES;
#endif

typedef struct tagThreadData {
	ppluint64	threadId;
#ifdef _WIN32
	HANDLE	thread;
	DWORD	dwThreadID;
#elif defined HAVE_PTHREADS
	pthread_t thread;
	pthread_attr_t	attr;
#else
	int	thread;
#endif
	void		(*mysql_thread_end)(struct tagThreadData *thread);
				// Bit  0: Thread hat MySQL benutzt
	void *clientData;
} THREADDATA;


typedef struct {
	Thread		*threadClass;
	void 		(*threadFunction)(void *);
	void 		*data;
	THREADDATA	*td;
} THREADSTARTUP;

#ifdef HAVE_TLS
	__thread THREADDATA *myThreadData=NULL;
#else
#ifdef HAVE_PTHREADS
static pthread_key_t thread_key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;
static void make_key()
{
	//DLOG ("static void make_key()\n");
	(void) pthread_key_create(&thread_key, NULL);
}
#endif
#endif


static void CreateTLS(THREADDATA *ptr)
{
#ifdef HAVE_TLS
	myThreadData=ptr;
#elif defined  HAVE_PTHREADS
	(void) pthread_once(&key_once, make_key);
	(void) pthread_setspecific(thread_key, myThreadData);
#elif defined  _WIN32
	Win32ThreadMutex.Lock();
	if (Win32ThreadTLS==TLS_OUT_OF_INDEXES) {
		Win32ThreadTLS=TlsAlloc();
		if (Win32ThreadTLS==TLS_OUT_OF_INDEXES) {
			::MessageBox(NULL,"TLS_OUT_OF_INDEXES","Error: ppl7::CreateTLS",MB_ICONERROR);
			exit(0);
		}
	}
	ptr->thread=GetCurrentProcess();
	if (!TlsSetValue(Win32ThreadTLS,ptr)) {
		::MessageBox(NULL,"TlsSetValue failed","Error: ppl6::GetThreadData",MB_ICONERROR);
		exit(0);
	}
	Win32ThreadMutex.Unlock();
#else
	throw NoThreadSupportException();
#endif
}


THREADDATA * GetThreadData()
/*!\ingroup PPLGroupThreads
 */
{
#if defined(HAVE_TLS) && defined(HAVE_PTHREADS)
	if (myThreadData==NULL) {
		myThreadData = new THREADDATA;
		memset(myThreadData,0,sizeof(THREADDATA));
		GlobalThreadMutex.lock();
		myThreadData->threadId=global_thread_id;
		global_thread_id++;
		GlobalThreadMutex.unlock();
		myThreadData->thread=pthread_self();
	}
	return myThreadData;
#elif defined  HAVE_PTHREADS
	(void) pthread_once(&key_once, make_key);
	THREADDATA *ptr;
	if ((ptr = (THREADDATA*)pthread_getspecific(thread_key)) == NULL) {
		// Nur der erste Thread kann hier landen, oder Threads die manuell ohne
		// die Thread-Klasse oder StartThread erstellt wurden
		ptr = new THREADDATA;
		memset(ptr,0,sizeof(THREADDATA));
		ptr->thread=pthread_self();
		GlobalThreadMutex.lock();
		ptr->threadid=global_thread_id;
		global_thread_id++;
		GlobalThreadMutex.unlock();
		pthread_attr_init(&ptr->attr);
		(void) pthread_setspecific(thread_key, ptr);
	}
	return ptr;
#elif defined  _WIN32
	THREADDATA *ptr;
	Win32ThreadMutex.lock();
	if (Win32ThreadTLS==TLS_OUT_OF_INDEXES) {
		Win32ThreadTLS=TlsAlloc();
		if (Win32ThreadTLS==TLS_OUT_OF_INDEXES) {
			::MessageBox(NULL,"TLS_OUT_OF_INDEXES","Error: ppl6::GetThreadData",MB_ICONERROR);
			exit(0);
		}
	}
	ptr=(THREADDATA*)TlsGetValue(Win32ThreadTLS);
	if (!ptr) {
		ptr = new THREADDATA;
		memset(ptr,0,sizeof(THREADDATA));
		ptr->thread=GetCurrentProcess();
		GlobalThreadMutex.lock();
		ptr->threadId=global_thread_id;
		global_thread_id++;
		GlobalThreadMutex.unlock();
		if (!TlsSetValue(Win32ThreadTLS,ptr)) {
			::MessageBox(NULL,"TlsSetValue failed","Error: ppl6::GetThreadData",MB_ICONERROR);
			exit(0);
		}
	}
	Win32ThreadMutex.unlock();
	return ptr;
#else
	throw NoThreadSupportException();
#endif
}




ppluint64 ThreadID()
{
	THREADDATA *ptr=GetThreadData();
	return ptr->threadId;

}

void *GetTLSData()
{
	THREADDATA *ptr=GetThreadData();
	return ptr->clientData;
}

void SetTLSData(void *data)
{
	THREADDATA *ptr=GetThreadData();
	ptr->clientData=data;
}


#ifdef _WIN32
	static DWORD WINAPI ThreadProc(void *param)
	{
		THREADSTARTUP *ts=(THREADSTARTUP *)param;
		CreateTLS(ts->td);
		if (ts->threadClass) {
			ts->threadClass->threadStartUp();
			if (ts->threadClass->threadShouldDeleteOnExit()) delete ts->threadClass;
		} else {
			ts->threadFunction(ts->data);
		}
		delete(myThreadData);
		myThreadData=NULL;
		return 0;
	}
#else
	static void *ThreadProc(void *param)
	{
		THREADSTARTUP *ts=(THREADSTARTUP *)param;
		CreateTLS(ts->td);
		if (ts->threadClass) {
			ts->threadClass->threadStartUp();
			if (ts->threadClass->threadShouldDeleteOnExit()) delete ts->threadClass;
		} else {
			ts->threadFunction(ts->data);
			pthread_attr_destroy(&ts->td->attr);
		}
		delete(myThreadData);
		myThreadData=NULL;
		pthread_exit(NULL);
		return NULL;
	}
#endif


ppluint64 StartThread(void (*start_routine)(void *),void *data)
{
	THREADSTARTUP ts;
	ts.threadClass=NULL;
	ts.threadFunction=start_routine;
	ts.data=data;
	ts.td=new THREADDATA;
	memset(ts.td,0,sizeof(THREADDATA));
	THREADDATA *t=ts.td;
	// ThreadId festlegen
	GlobalThreadMutex.lock();
	t->threadId=global_thread_id;
	global_thread_id++;
	GlobalThreadMutex.unlock();
#ifdef _WIN32
	t->thread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadProc,&ts,0,&t->dwThreadID);
	if (t->thread!=NULL) {
		return t->threadId;
	}
	throw ThreadStartException();
#elif defined HAVE_PTHREADS
	pthread_attr_init(&t->attr);
	int ret=pthread_create(&t->thread,&t->attr,ThreadProc,&ts);
	if(ret==0) {
		pthread_detach(t->thread);
		return t->threadId;
	}
	throw ThreadStartException();
#else
	throw NoThreadSupportException();
#endif
	return true;
}

/*! \brief Priorität des aktuellen Threads ändern
 * \ingroup PPLGroupThreads
 *
 * \ingroup PPLGroupThreadsPriority
 *
 */
void ThreadSetPriority(Thread::Priority priority)
{
#ifdef WIN32
	HANDLE h=GetCurrentProcess();
	int p=GetThreadPriority(h);
	switch(priority) {
		case Thread::LOWEST:
			p=THREAD_PRIORITY_LOWEST;
			break;
		case Thread::BELOW_NORMAL:
			p=THREAD_PRIORITY_BELOW_NORMAL;
			break;
		case Thread::NORMAL:
			p=THREAD_PRIORITY_NORMAL;
			break;
		case Thread::ABOVE_NORMAL:
			p=THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case Thread::HIGHEST:
			p=THREAD_PRIORITY_HIGHEST;
			break;
		default:
			throw IllegalArgumentException();
	}
	if (!SetThreadPriority(h,p)) throw ThreadOperationFailedException();
#elif defined HAVE_PTHREADS
	struct sched_param s;
	pthread_t p=pthread_self();
	int policy,c;
	c=pthread_getschedparam(p,&policy,&s);
	if (c!=0) throw ThreadOperationFailedException();
	int min=sched_get_priority_min(policy);
	int max=sched_get_priority_max(policy);
	int normal=(min+max)/2;
	switch(priority) {
		case Thread::LOWEST:
			s.sched_priority=min;
			break;
		case Thread::BELOW_NORMAL:
			s.sched_priority=normal/2;
			break;
		case Thread::NORMAL:
			s.sched_priority=normal;
			break;
		case Thread::ABOVE_NORMAL:
			s.sched_priority=normal+normal/2;
			break;
		case Thread::HIGHEST:
			s.sched_priority=max;
			break;
		default:
			throw IllegalArgumentException();
	}
	c=pthread_setschedparam(p,policy,&s);
	if(c!=0) throw ThreadOperationFailedException();
#else
	throw NoThreadSupportException();
#endif
}

/*! \brief Priorität des aktuellen Threads abfragen
 * \ingroup PPLGroupThreads
 *
 * \ingroup PPLGroupThreadsPriority
 */
Thread::Priority ThreadGetPriority()
{
#ifdef WIN32
	HANDLE h=GetCurrentProcess();
	int p=GetThreadPriority(h);
	switch(p) {
		case THREAD_PRIORITY_LOWEST:
			return Thread::LOWEST;
		case THREAD_PRIORITY_BELOW_NORMAL:
			return Thread::BELOW_NORMAL;
		case THREAD_PRIORITY_NORMAL:
			return Thread::NORMAL;
		case THREAD_PRIORITY_ABOVE_NORMAL:
			return Thread::ABOVE_NORMAL;
		case THREAD_PRIORITY_HIGHEST:
			return Thread::HIGHEST;
	}
	return Thread::UNKNOWN;
#elif defined HAVE_PTHREADS
	struct sched_param s;
	pthread_t p=pthread_self();
	int policy,c;
	c=pthread_getschedparam(p,&policy,&s);
	if(c!=0) throw ThreadOperationFailedException();
	int min=sched_get_priority_min(policy);
	int max=sched_get_priority_max(policy);
	int normal=(min+max)/2;

	if (s.sched_priority==normal) return Thread::NORMAL;
	if (s.sched_priority==min) return Thread::LOWEST;
	if (s.sched_priority==max) return Thread::HIGHEST;
	if (s.sched_priority<normal) return Thread::BELOW_NORMAL;
	if (s.sched_priority>normal) return Thread::ABOVE_NORMAL;
	return Thread::UNKNOWN;
#else
	throw NoThreadSupportException();
#endif
}


/*!\class Thread
 * \ingroup PPLGroupThreads
 * \brief Klasse zum Verwalten von Threads
 *
 * Klasse zum Starten und Verwalten von Threads.
 * \see \ref PPLGroupThreads
 * \par Beispiel:
 * \include Thread_ThreadMain.cpp
 *
 */

#ifdef DONE

Thread::Thread()
/*! \brief Konstruktor der Thread-Klasse
 *
 * Konstruktor der Thread-Klasse. Es werden interne Variablen allokiert und mit
 * Default-Werten gefüllt.
 *
 * \see \ref PPLGroupThreads
 */
{
	threadId=0;
	flags=0;
	myPriority=Thread::NORMAL;
	THREADDATA *t=new THREADDATA;
	memset(t,0,sizeof(THREADDATA));
	threaddata=t;
	t->thread=0;
	IsRunning=0;
	IsSuspended=0;
	deleteMe=0;
	#ifdef HAVE_PTHREADS
		pthread_attr_init(&t->attr);
	#endif
}

Thread::~Thread()
/*! \brief Destruktor der Thread-Klasse
 *
 * Falls der Thread noch läuft, wird er zunächst gestoppt. Anschließend werden die
 * internen Variablen wieder freigegeben.
 *
 * \see \ref PPLGroupThreads
 */
{
	threadStop();
	THREADDATA *t=(THREADDATA *)threaddata;
	#ifdef HAVE_PTHREADS
		pthread_attr_destroy(&t->attr);
	#endif
	delete t;
}

void Thread::threadSuspend()
/*! \brief Der Thread soll pausieren
 *
 * ThreadSuspend setzt das Suspended Flag. Hat nur Auswirkungen, wenn dieses Flag in ThreadMain
 * beachtet wird.
 *
 * \todo Es wäre besser, wenn diese Funktion den Thread Betriebssystemseitig schlafen legen
 * würde, bis ein Resume gegeben wird.
 *
 * \see Thread::ThreadResume
 * \see Thread::ThreadWaitSuspended
 * \see \ref PPLGroupThreads
 */
{
	threadmutex.lock();
	flags|=2;
	threadmutex.unlock();
}

void Thread::threadResume()
/*! \brief Der Thread soll weitermachen
 *
 * Dem Thread wird signalisiert, daß er weitermachen soll.
 *
 * \todo Es wäre besser, wenn diese Funktion vom Betriebssystemseitig erledigt würde.
 *
 * \see Thread::ThreadSuspend
 * \see Thread::ThreadWaitSuspended
 * \see \ref PPLGroupThreads
 */
{
	threadmutex.lock();
	flags=flags&~2;
	if (IsSuspended) {
		threadmutex.unlock();
		threadmutex.signal();
	} else {
		threadmutex.unlock();
	}
}

void Thread::threadStop()
/*! \brief Der Thread wird gestoppt
 *
 * Dem Thread wird zunächst signalisiert, dass er stoppen soll. Anschließend wartet die
 * Funktion, bis der Thread sich beendet hat.
 *
 * \note Die Thread-Funktion muß in regelmäßigen Abständen mittels der Funktion
 * ThreadShouldStop überprüfen, ob er stoppen soll. Ist dies der Fall, muß sich die
 * Funktion beenden.
 *
 * \see Thread::ThreadSignalStop
 * \see Thread::ThreadShouldStop
 * \see \ref PPLGroupThreads
 */
{
	threadmutex.lock();
	flags|=1;
	//THREADDATA *t=(THREADDATA *)threaddata;
	if (IsSuspended) {
		threadmutex.unlock();
		threadResume();
		threadmutex.lock();
	}
	while (IsRunning) {
		threadmutex.unlock();
		MSleep(1);
		threadmutex.lock();
	}
	flags=flags&0xfffffffe;
	threadmutex.unlock();
}

void Thread::threadSignalStop()
/*! \brief Dem Thread signalisieren, dass er stoppen soll
 *
 * Dem Thread wird nur signalisiert, dass er stoppen soll.
 *
 * \see Thread::ThreadStop
 * \see Thread::ThreadShouldStop
 * \see \ref PPLGroupThreads
 */
{
	threadmutex.lock();
	flags|=1;
	//THREADDATA *t=(THREADDATA *)threaddata;
	if (IsSuspended) {
		threadmutex.unlock();
		threadResume();
	} else {
		threadmutex.unlock();
	}
}

void Thread::threadStart()
/*! \brief Der Thread wird gestartet
 *
 * ThreadStart startet den Thread und kehrt sofort zur aufrufenden Funktion zurück.
 *
 * \param param Ein optionaler Pointer auf eine Datenstruktur, der an die ThreadMain-Funktion
 * übergeben wird.
 * \return Konnte der Thread erfolgreich gestartet werden, wird 1 zurückgegeben, bei einem Fehler 0.

 *
 * \see Thread::ThreadMain
 * \see \ref PPLGroupThreads
 */
{

	THREADDATA *t=(THREADDATA *)threaddata;
	threadmutex.lock();
	IsRunning=1;
	IsSuspended=0;
	threadmutex.unlock();
	#ifdef _WIN32
		t->thread=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)ThreadProc,this,NULL,&t->dwThreadID);
		if (t->thread!=NULL) {
			return 1;
		}
		threadmutex.Lock();
		IsRunning=0;
		threadmutex.Unlock();

	#elif defined HAVE_PTHREADS
		int ret=pthread_create(&t->thread,&t->attr,ThreadProc,t);
		if(ret==0) {
			if (pthread_detach(t->thread)==0) return 1;
			// TODO: Wenn pthread_detach fehlschlägt, kann der Thread trotzdem
			// laufen
		}
		threadmutex.Lock();
		IsRunning=0;
		threadmutex.Unlock();
	#else

	#endif
	return 0;
}

void Thread::ThreadIdle()
/*!\brief Der Thread gibt seine CPU-Zeit an andere Threads ab
 */
{
#ifdef _WIN32
#elif defined HAVE_PTHREADS
	#ifdef SOLARIS
	#else
		pthread_yield();
	#endif
#endif
}

void Thread::ThreadStartUp()
/*! \brief Interne Funktion
 *
 * ThreadStartUp wird unmittelbar nach Starten des Threads aufgerufen. Hier werden einige
 * Variablen initialisiert und dann ThreadMain aufgerufen.
 *
 * \note Diese Funktion wird intern verwendet und sollte nicht vom Anwender aufgerufen
 * werden
 *
 * \see Thread::ThreadMain
 * \see \ref PPLGroupThreads
 */
{
	threadmutex.Lock();
	threadid=GetThreadID();
	IsRunning=1;
	IsSuspended=0;
	threadmutex.Unlock();
	ThreadSetPriority(myPriority);

	THREADDATA * d=GetThreadData();

	ThreadMain(param);


	if (d) {
#ifdef HAVE_MYSQL
		if (d->mysql_thread_end!=NULL) d->mysql_thread_end(d);
#endif

	}

	threadmutex.Lock();
	flags=0;
	IsRunning=0;
	IsSuspended=0;
	//MemoryDebugRemoveThread();
	threadmutex.Unlock();
}

void Thread::ThreadDeleteOnExit(int flag)
/*! \brief Flag setzen: Klasse beim Beenden löschen
 *
 * Dem Thread wird mitgeteilt, ob er beim beenden seine eigene Klasse löschen soll. Der
 * Default ist, dass der Anwender selbst die Klasse löschen muß.
 *
 * \param flag kann entweder 1 (Klasse soll automatisch gelöscht werden) oder 0 sein
 * (Klasse nicht löschen).
 * \see Thread::ThreadDeleteOnExit
 * \see \ref PPLGroupThreads
 */
{
	if (flag) deleteMe=1;
	else deleteMe=0;
}

int  Thread::ThreadShouldDeleteOnExit()
/*! \brief Interne Funktion
 *
 * Diese Funktion wird intern beim beenden des Threads aufgerufen. Liefert sie "true" zurück,
 * wird die Thread-Klasse automatisch mit delete gelöscht.
 *
 * \return Liefert 1 zurück, wenn die Klasse gelöscht werden soll, sonst 0.
 * \see Thread::ThreadDeleteOnExit
 * \see \ref PPLGroupThreads
 */
{
	if (deleteMe) return 1;
	return 0;
}

int Thread::ThreadIsRunning()
/*! \brief Status abfragen: Läuft der Thread?
 *
 * Mit dieser Funktion kann überprüft werden, ob der Thread aktuell ausgeführt wird.
 *
 * \return Liefert 1 zurück, wenn der Thread läuft, sonst 0.
 * \see \ref PPLGroupThreads
 */
{
	int ret;
	threadmutex.Lock();
	ret=IsRunning;
	threadmutex.Unlock();
	return ret;
}

int Thread::ThreadIsSuspended()
/*! \brief Status abfragen: Schläft der Thread?
 *
 * Mit dieser Funktion kann überprüft werden, ob der Thread aktuell schläft.
 *
 * \return Liefert 1 zurück, wenn der Thread schläft, sonst 0.
 * \see \ref PPLGroupThreads
 */
{
	int ret;
	threadmutex.Lock();
	ret=IsSuspended;
	threadmutex.Unlock();
	return ret;
}

int Thread::ThreadGetFlags()
/*! \brief Flags des Thread auslesen
 *
 * Mit dieser Funktion können die internen Flags ausgelesen werden.
 *
 * \return Liefert den Wert der internen Flag-Variable zurück
 * \deprecated
 * Diese Funktion ist veraltet und sollte nicht mehr verwendet werden.
 * \see \ref PPLGroupThreads
 */
{
	int ret;
	threadmutex.Lock();
	ret=flags;
	threadmutex.Unlock();
	return ret;
}

int Thread::ThreadShouldStop()
/*! \brief Prüfen, ob der Thread beendet werden soll
 *
 * Diese Funktion liefert \c true zurück, wenn der Thread gestoppt werden soll.
 * Dies ist der Fall, wenn vorher die Funktion ThreadStop oder ThreadShouldStop
 * aufgerufen wurde.
 *
 * \return Liefert 1 zurück, wenn der Thread gestoppt werden soll, sonst 0.
 * \see Thread::ThreadStop
 * \see Thread::ThreadShouldStop
 * \see \ref PPLGroupThreads
 */
{
	int ret;
	threadmutex.Lock();
	ret=flags&1;
	threadmutex.Unlock();
	return ret;
}

void Thread::ThreadWaitSuspended(int msec)
/*! \brief Prüfen, ob der Thread schlafen soll
 *
 * ThreadWaitSuspended prüft, ob der Thread schlafen (suspend) soll, und wenn
 * ja, wartet sie solange, bis ein unsuspend oder stop signalisiert wird.
 * Der optionale Parameter gibt an, nach wievielen Millisekunden jeweils der Status
 * geprüft werden soll.
 *
 * \param msec Millisekunden, nach denen jeweils der Status geprüft werden soll.
 * Wird der Parameter nicht angegeben, wartet die Funktion so lange, bis entweder
 * die Funktion ThreadResume, TheadSignalStop, ThreadStop oder der Destruktor
 * der Klasse aufgerufen wird.
 *
 * \see Thread::ThreadSuspend
 * \see Thread::ThreadResume
 * \see \ref PPLGroupThreads
 */
{
	threadmutex.Lock();
	//THREADDATA *t=(THREADDATA *)threaddata;
	while ((flags&3)==2) {
		IsSuspended=1;
		threadmutex.Wait(msec);
	}
	IsSuspended=0;
	threadmutex.Unlock();
}

ppluint64 Thread::ThreadGetID()
/*! \brief ThreadID zurückgeben
 *
 * Diese Funktion liefert die interne ID des Threads zurück.
 *
 * \return Liefert einen 64-Bit-Wert mit der Thread-ID zurück.
 * \see \ref PPLGroupThreads
 */
{
	THREADDATA *t=(THREADDATA *)threaddata;
	if (!t) return 0;
	if (!t->threadid) {
		t->threadid=GetThreadID();
	}
	return t->threadid;
}


void Thread::ThreadMain(void *param)
/*!\brief Einsprungfunktion bei Start des Threads
 *
 * ThreadMain ist die Funktion, die nach Starten des Threads aufgerufen wird.
 * Sie muß von der abgeleiteten Klasse überschrieben werden und enthält den vom
 * Thread auszuführenden Code.
 *
 * \param param Ein optionaler Void-Pointer, der mit der Funktion ThreadStart
 * übergeben wurde.
 * \return Die Funktion liefert keinen Return-Wert, jedoch wird bei Verlassen
 * der Thread automatisch gestoppt. Wurde zuvor die Funktion Thread::ThreadShouldDeleteOnExit()
 * aufgerufen, wird außerdem die Klasse gelöscht.
 * \see \ref PPLGroupThreads
 * \par Example
 * \include Thread_ThreadMain.cpp
 */
{

}

int Thread::ThreadGetPriority()
/*! \brief Priorität des Threads auslesen
 * \ingroup PPLGroupThreadsPriority
 *
 * Liefert die Priorität des Threads zurück.
 *
 * \return liefert einen Wert zurück, der die Priorität des Threads angibt.
 * \see \ref PPLGroupThreads
 */
{
	THREADDATA *t=(THREADDATA *)threaddata;
#ifdef WIN32
	int p=GetThreadPriority(t->thread);
	switch(p) {
		case THREAD_PRIORITY_LOWEST:
			return THREAD_PRIORITY::LOWEST;
		case THREAD_PRIORITY_BELOW_NORMAL:
			return THREAD_PRIORITY::BELOW_NORMAL;
		case THREAD_PRIORITY_NORMAL:
			return THREAD_PRIORITY::NORMAL;
		case THREAD_PRIORITY_ABOVE_NORMAL:
			return THREAD_PRIORITY::ABOVE_NORMAL;
		case THREAD_PRIORITY_HIGHEST:
			return THREAD_PRIORITY::HIGHEST;
	}
#elif defined HAVE_PTHREADS
	struct sched_param s;
	int policy,c;
	c=pthread_getschedparam(t->thread,&policy,&s);
	if(c!=0) return 0;
	int min=sched_get_priority_min(policy);
	int max=sched_get_priority_max(policy);
	int normal=(min+max)/2;

	if (s.sched_priority==normal) return THREAD_PRIORITY::NORMAL;
	if (s.sched_priority==min) return THREAD_PRIORITY::LOWEST;
	if (s.sched_priority==max) return THREAD_PRIORITY::HIGHEST;
	if (s.sched_priority<normal) return THREAD_PRIORITY::BELOW_NORMAL;
	if (s.sched_priority>normal) return THREAD_PRIORITY::ABOVE_NORMAL;
	return THREAD_PRIORITY::UNKNOWN;

#else
	return THREAD_PRIORITY::UNKNOWN;
#endif

	return THREAD_PRIORITY::UNKNOWN;
}

int Thread::ThreadSetPriority(int priority)
/*! \brief Priorität des Threads ändern
 * \ingroup PPLGroupThreadsPriority
 *
 * Setz die Priorität des Threads
 * \param priority Gibt die Priorität des Threads an. Die möglichen Werte sind im
 * Kapitel \link PPLGroupThreadsPriority Thread Prioritäten \endlink beschrieben.
 * \return Liefert 1 zurück, wenn die Priorität erfolgreich geändert wurde, sonst 0.
 * \see \ref PPLGroupThreads
 */
{
	THREADDATA *t=(THREADDATA *)threaddata;
	myPriority=priority;
	if(!t->thread) return 1;
#ifdef WIN32
	int p=GetThreadPriority(t->thread);
	switch(priority) {
		case THREAD_PRIORITY::LOWEST:
			p=THREAD_PRIORITY_LOWEST;
			break;
		case THREAD_PRIORITY::BELOW_NORMAL:
			p=THREAD_PRIORITY_BELOW_NORMAL;
			break;
		case THREAD_PRIORITY::NORMAL:
			p=THREAD_PRIORITY_NORMAL;
			break;
		case THREAD_PRIORITY::ABOVE_NORMAL:
			p=THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case THREAD_PRIORITY::HIGHEST:
			p=THREAD_PRIORITY_HIGHEST;
			break;
	}
	if (SetThreadPriority(t->thread,p)) return 1;
	return 0;
#elif defined HAVE_PTHREADS
	struct sched_param s;
	int policy,c;
	c=pthread_getschedparam(t->thread,&policy,&s);
	if (c!=0) return 0;
	int min=sched_get_priority_min(policy);
	int max=sched_get_priority_max(policy);
	int normal=(min+max)/2;
	switch(priority) {
		case THREAD_PRIORITY::LOWEST:
			s.sched_priority=min;
			break;
		case THREAD_PRIORITY::BELOW_NORMAL:
			s.sched_priority=normal/2;
			break;
		case THREAD_PRIORITY::NORMAL:
			s.sched_priority=normal;
			break;
		case THREAD_PRIORITY::ABOVE_NORMAL:
			s.sched_priority=normal+normal/2;
			break;
		case THREAD_PRIORITY::HIGHEST:
			s.sched_priority=max;
			break;
		default:
			return 0;
	}
	c=pthread_setschedparam(t->thread,policy,&s);
	if(c==0) return 1;
	return 0;
#else
	return 0;
#endif

}

int Thread::ThreadSetStackSize(size_t size)
/*! \brief Stack-Größe des Threads setzen
 * \ingroup PPLGroupThreadsStacksize
 *
 * \see \ref PPLGroupThreadsStacksize
 * \see \ref PPLGroupThreads
 */
{
	#ifdef HAVE_PTHREADS
		#ifndef _POSIX_THREAD_ATTR_STACKSIZE
			SetError(315);
			return 0;
		#endif
		THREADDATA *t=(THREADDATA *)threaddata;
		if (size==0) size=PTHREAD_STACK_MIN;
		if (size<PTHREAD_STACK_MIN) {
			SetError(314,PTHREAD_STACK_MIN,"Stacksize muss mindestens %u Byte sein",PTHREAD_STACK_MIN);
			return 0;
		}
		if (pthread_attr_setstacksize(&t->attr,size)==0) return 1;
	#endif
	return 0;
}

size_t Thread::ThreadGetMinimumStackSize()
/*! \brief Minimale Stack-Größe auslesen
 * \ingroup PPLGroupThreadsStacksize
 *
 * \see \ref PPLGroupThreadsStacksize
 * \see \ref PPLGroupThreads
 */
{
	#ifdef HAVE_PTHREADS
		#ifndef _POSIX_THREAD_ATTR_STACKSIZE
			SetError(315);
			return 0;
		#endif
		return PTHREAD_STACK_MIN;
		#endif
	return 0;
}

size_t Thread::ThreadGetStackSize()
/*! \brief Stack-Größe des Threads auslesen
 * \ingroup PPLGroupThreadsStacksize
 *
 * \see \ref PPLGroupThreadsStacksize
 * \see \ref PPLGroupThreads
 */
{
	#ifdef HAVE_PTHREADS
		#ifndef _POSIX_THREAD_ATTR_STACKSIZE
			SetError(315);
			return 0;
		#endif
		THREADDATA *t=(THREADDATA *)threaddata;
		size_t s;
		if (pthread_attr_getstacksize(&t->attr,&s)==0) return s;
	#endif
	return 0;
}

#endif

} // EOF namespace ppl7
