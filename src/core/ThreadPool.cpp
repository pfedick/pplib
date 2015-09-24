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
 * Copyright (c) 2015, Patrick Fedick <patrick@pfp.de>
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

namespace ppl7 {

void ThreadPool::addThread(ppl7::Thread *thread)
{
	std::pair<std::set<ppl7::Thread*>::iterator, bool> ret;
	ret = threads.insert(thread);
	if (ret.second == false)
		throw ppl7::ThreadAlreadyInPoolException();
}

void ThreadPool::removeThread(ppl7::Thread *thread)
{

}

void ThreadPool::destroyThread(ppl7::Thread *thread)
{

}

void ThreadPool::clear(bool destroyThreads)
{
	if (destroyThreads) {
		std::set<ppl7::Thread*>::iterator it;
		for (it = threads.begin(); it != threads.end(); ++it) {
			delete (*it);
		}
	}
	threads.clear();
}

ThreadPool::iterator ThreadPool::begin()
{
	return threads.begin();
}

ThreadPool::const_iterator ThreadPool::begin() const
{
	return threads.begin();
}

ThreadPool::iterator ThreadPool::end()
{
	return threads.end();
}

ThreadPool::const_iterator ThreadPool::end() const
{
	return threads.end();
}

void ThreadPool::signalStopThreads()
{

}

void ThreadPool::stopThreads()
{

}

size_t ThreadPool::size() const
{
	return threads.size();
}

bool ThreadPool::running() const
{
	std::set<ppl7::Thread*>::const_iterator it;
	for (it = threads.begin(); it != threads.end(); ++it) {
		if ((*it)->threadIsRunning())
			return true;
	}
	return false;
}

}	// EOF namespace ppl7
