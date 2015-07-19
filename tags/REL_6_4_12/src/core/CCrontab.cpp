/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: CCrontab.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
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
#include "ppl6.h"


namespace ppl6 {

/*!\class CCronjob
 * \ingroup PPLGroupDateTime
 * \brief Einzelner Timer für die Klasse ppl6::CCrontab
 */

CCronjob::CCronjob()
{
	crontab=NULL;
	intervall=0;
	nextevent=0;
	callback=NULL;
	callback_data=NULL;
}

CCronjob::~CCronjob()
{
	if (crontab) crontab->Delete(this);
}

void CCronjob::StartOnce(int ms)
{
	intervall=0;
	nextevent=ppl6::getmicrotime()+(double)ms/1000.0;
}

void CCronjob::StartIntervall(int ms)
{
	intervall=(double)ms/1000.0;
	nextevent=ppl6::getmicrotime()+intervall;
}
void CCronjob::SetCallback(CCallback *c, void *data)
{
	callback=c;
	callback_data=data;
}

/*!\class CCrontab
 * \ingroup PPLGroupDateTime
 * \brief Crontab zur steuerung von Timmer-Events
 */


CCrontab::CCrontab()
{
}

CCrontab::~CCrontab()
{
	Stop();
}

int CCrontab::Start()
{
	if (ThreadIsRunning()) return 1;
	return ThreadStart();
}

int CCrontab::Stop()
{
	if (!ThreadIsRunning()) return 1;
	ThreadSignalStop();
	ThreadStop();
	return 1;
}

int CCrontab::Add(CCronjob *job)
{
	if (!job) {
		SetError(194,"int CCrontab::Delete(CCronjob *job)");
		return 0;
	}
	Mutex.Lock();
	if (!jobs.Add(job)) {
		PushError();
		Mutex.Unlock();
		PopError();
		return 0;
	}
	job->crontab=this;
	Mutex.Unlock();
	return 1;
}

int CCrontab::Delete(CCronjob *job)
{
	if (!job) {
		SetError(194,"int CCrontab::Delete(CCronjob *job)");
		return 0;
	}
	Mutex.Lock();
	if (job->crontab!=this) {
		Mutex.Unlock();
		return 0;
	}
	job->crontab=NULL;
	if (!jobs.Delete(job)) {
		PushError();
		Mutex.Unlock();
		PopError();
		return 0;
	}
	Mutex.Unlock();
	return 1;
}

void CCrontab::ThreadMain(void *param)
{
	double now, diff;
	ppluint64 s=10;
	CCronjob *job;
	while (!ThreadShouldStop()) {
		Mutex.Lock();
		now=ppl6::getmicrotime();
		jobs.Reset();
		while ((job=(CCronjob*)jobs.GetNext())) {
			if (job->nextevent>0 && job->nextevent<=now) {
				if (job->intervall) job->nextevent=now+job->intervall;
				else job->nextevent=0;
				if (job->nextevent) {
					diff=job->nextevent-now;
					if (diff*1000<s) s=(ppluint64)(diff*1000.0);
				}
				if (job->callback) {
					Mutex.Unlock();
					job->callback->Callback(job->callback_data);
					Mutex.Lock();
				}
			}
		}
		Mutex.Unlock();
		MSleep(s);
	}
}


}
