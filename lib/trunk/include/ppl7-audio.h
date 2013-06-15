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

#ifndef _PPL7_INCLUDE_AUDIO
#define _PPL7_INCLUDE_AUDIO
#ifndef _PPL7_INCLUDE
    #ifdef PPL7LIB
        #include "ppl7.h"
    #else
        #include <ppl7.h>
    #endif
#endif


#ifndef LPVOID
#define LPVOID void*
#endif

namespace ppl7 {

class Icecast
{
	private:
		void	*shout;
		bool    bconnected;
	public:
		Icecast();
		~Icecast();
		String	getVersion(int *major=NULL, int *minor=NULL, int *patch=NULL) const;
		String version() const;
		bool	connected();
		void	setConnection(const String &host, int port, const String &password);
		String host() const;
		int		port() const;
		String	password() const;
		void	setMount(const String &mount);
		String mount() const;
		void	setName(const String &name);
		String name() const;
		void	setUrl(const String &url);
		String url() const;
		void	setGenre(const String &genre);
		String genre() const;
		void	setUser(const String &user);
		String user() const;
		void	setAgent(const String &agent);
		String agent() const;
		void	setDescription(const String &description);
		String description() const;
		void	setDumpfile(const String &file);
		String dumpfile() const;
		void	setAudioInfo(const String &name, const String &value);
		String audioInfo(const String &name) const;
		void	setPublic(bool makepublic);
		void	setFormatMP3();
		void	setFormatOGG();
		void	setNonBlocking(bool flag);

		void	connect();
		void	disconnect();

		void	send(const void *buffer, size_t bytes);
		int		delay();
		void	sync();

		void	sendMetadata(const String &name, const String &value);
		void	setTitle(const String &title);

};





} // end of namespace ppl7


#endif // _PPL6_INCLUDE_SOUND

