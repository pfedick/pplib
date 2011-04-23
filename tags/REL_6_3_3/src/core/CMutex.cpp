/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: patrick $
 * $Revision: 1.6 $
 * $Date: 2008/04/15 20:36:33 $
 * $Id: CMutex.cpp,v 1.6 2008/04/15 20:36:33 patrick Exp $
 * 
 ******************************************************************************* 
 * Copyright (c) 2008, Patrick Fedick <patrick@pfp.de>
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

#ifdef HAVE_STRINGS_H
	#include <strings.h>
#endif
#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif
#ifdef HAVE_PTHREADS
	#include <pthread.h>
#endif

#ifdef HAVE_ERRNO_H
	#include <errno.h>
#endif

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
	#include <sys/time.h>
#endif


#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN		// Keine MFCs

	// ms-help://MS.VSCC.2003/MS.MSDNQTR.2003FEB.1031/sdkintro/sdkintro/using_the_sdk_headers.htm
	#ifndef _WIN32_WINNT
	#   define _WIN32_WINNT 0x500
	#endif
	#include <windows.h>
#endif	


#include "ppl6.h"

namespace ppl6 {
	
typedef struct tagMutex
{
	#ifdef _WIN32
		HANDLE	handle;
	#elif defined HAVE_PTHREADS
		pthread_mutex_t handle;
		pthread_cond_t	condition;
	#else
		void *handle;
	#endif
} PPLMUTEX;

/*!\class CMutex
 * \ingroup PPLGroupThreads
 * \ingroup PPLGroupThreadsMutex
 * 
 * Mutexes stellen ein Low-Level-Interface für die Synchronisation von Threads dar. Mit ihnen
 * kann verhindert werden, dass mehrere Threads versuchen gleichzeitig auf die gleichen Daten
 * zuzugreifen.
 * 
 */
 
CMutex::CMutex()
{
	handle=::malloc(sizeof(PPLMUTEX));
	if (!handle) return;
	PPLMUTEX *h=(PPLMUTEX*)handle;
	#ifdef _WIN32
		h->handle=CreateMutex(NULL,false,NULL);
	#elif defined HAVE_PTHREADS
		pthread_mutex_init(&h->handle,NULL);
		pthread_cond_init(&h->condition,NULL);
	#else 
		h->handle=NULL
	#endif

}

CMutex::~CMutex()
{
	if (handle==NULL) return;
	PPLMUTEX *h=(PPLMUTEX*)handle;
	#ifdef _WIN32
		CloseHandle(h->handle);
	#elif defined HAVE_PTHREADS
		pthread_mutex_destroy(&h->handle);
		int rc=pthread_cond_destroy(&h->condition);
		if (rc == EBUSY) {	// Thread warten noch auf Condition-Variable
			pthread_cond_signal(&h->condition);
		}		
	#endif
	::free(handle);
	handle=NULL;
}

int CMutex::Lock()
/*!\brief Mutex sperren
 * 
 * Diese Funktion versucht einen Mutex zu sperren. Ist dieser bereits durch einen
 * anderen Thread blockiert, wird gewartet, bis der Mutex wieder frei wird.
 * 
 * \return Konnte der Mutex erfolgreich gesperrt werden, liefert die Funktion
 * true (1) zurück, im Fehlerfall false (0)
 */
{
	if (!handle) return 0;
	PPLMUTEX *h=(PPLMUTEX*)handle;
	#ifdef _WIN32
		int ret=WaitForSingleObject(h->handle,INFINITE);
		if (ret!=WAIT_FAILED) return 1;
		return 0;
	#elif defined HAVE_PTHREADS
		int ret=pthread_mutex_lock(&h->handle);
		if (ret==0) return 1;
		return 0;
	#else
		return 0;
	#endif
}

int CMutex::Unlock()
/*!\brief Mutex entsperren
 * 
 * Mit dieser Funktion wird ein zuvor mit CMutex::Lock gesperrter
 * Mutex wieder frei gegeben.
 *  
 * \return Konnte der Mutex erfolgreich entsperrt werden, liefert die Funktion
 * true (1) zurück, im Fehlerfall false (0)
 */
{
	if (!handle) return 0;
	PPLMUTEX *h=(PPLMUTEX*)handle;
	#ifdef _WIN32
		int ret=ReleaseMutex(h->handle);
		if (ret==0) return 0;
		return 1;
	#elif defined HAVE_PTHREADS
		int ret=pthread_mutex_unlock(&h->handle);
		if (ret==0) return 1;
		return 0;
	#else
		return 0;
	#endif
}

int CMutex::TryLock()
/*!\brief Mutex versuchen zu sperren
 * 
 * Diese Funktion versucht wie CMutex::Lock einen Mutex zu sperren.
 * Ist dieser allerdings bereits durch einen anderen Thread blockiert,
 * wird nicht gewartet, bis der Mutex wieder frei wird, sondern es wird
 * sofort ein Fehler zurückgeliefert.
 * 
 * \return Konnte der Mutex erfolgreich gesperrt werden, liefert die Funktion
 * true (1) zurück, im Fehlerfall false (0)
 */
{
	if (!handle) return 0;
	PPLMUTEX *h=(PPLMUTEX*)handle;
	#ifdef _WIN32
		int ret=WaitForSingleObject(h->handle,0);
		if (ret==WAIT_TIMEOUT) return 0;
		if (ret!=WAIT_FAILED) return 1;
		return 0;
	#elif defined HAVE_PTHREADS
		int ret=pthread_mutex_trylock(&h->handle);
		if (ret==0) return 1;
		return 0;
	#else
		return 0;
	#endif
}

int CMutex::Wait(int microseconds)
/*! \brief Auf Signal warten
 * 
 * Der aufrufende Thread wird angehalten, bis ein anderer Thread diesem Mutex
 * ein Signal gibt (siehe CMutex::Signal) oder die optional angegebene Zeit
 * abgelaufen ist.
 * 
 * \param microseconds Optionale Angabe einer Zeit, nach deren Ablauf der Thread
 * automatisch wieder fortgeführt wird. Default = 0 = unendlich.
 * 
 * \return Liefert true (1) bei Erfolg, false (0) bei Fehlschlag
 * 
 * War der Mutex vor Aufruf dieser Funktion bereits gelocked, ist er es nach
 * Ende immer noch.
 */
{
	int releaseatend=0;
	if (!handle) return 0;
	PPLMUTEX *h=(PPLMUTEX*)handle;
	#ifdef _WIN32
		return 0;
	#elif defined HAVE_PTHREADS
		int ret=pthread_mutex_trylock(&h->handle);
		if (ret==0) {
			releaseatend=1;
		} else if(ret==EBUSY) {	// Mutex ist schon gelocked
			releaseatend=0;
		} else {
			return 0;
		}
			
		if (microseconds>0) {
			struct timeval now;
			struct timespec timeout;
			// Get current Time
			gettimeofday(&now,NULL);
			int s=microseconds/1000;
			int m=microseconds%1000;
			int n=(m*1000)+now.tv_usec;
			s+=n/1000000;
			timeout.tv_sec=now.tv_sec + s;
			timeout.tv_nsec=n%1000000;
			ret=pthread_cond_timedwait(&h->condition,&h->handle,&timeout);
		} else {
			ret=pthread_cond_wait(&h->condition,&h->handle);
		}
		if (releaseatend) pthread_mutex_unlock(&h->handle);
		if (ret==0) {
			return 1;
		}
		return 0;
	#else
		return 0;
	#endif	
}


int CMutex::Signal()
/*! \brief Signal senden
 * 
 * Mit dieser Funktion wird dem Mutex ein Signal gegeben. Hat ein Thread sich selbst
 * durch Aufruf der Wait-Funktion (siehe CMutex::Wait) angehalten, wird er
 * nach Aufruf des Signals fortgesetzt.
 * 
 * \return Bei Erfolg liefert die Funktion true (1) zurück, sonst false (0).
 */ 
{
	if (!handle) return 0;
	PPLMUTEX *h=(PPLMUTEX*)handle;
	#ifdef _WIN32
		return 0;
	#elif defined HAVE_PTHREADS
		int ret=pthread_cond_signal(&h->condition);
		if (ret==0) return 1;
		return 0;
	#else
		return 0;
	#endif
}


} // EOF namespace ppl
