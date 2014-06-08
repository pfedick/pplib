/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
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
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif

#ifdef HAVE_LIBCDDB
#include <cddb/cddb.h>
#endif

#include "ppl6.h"
#include "ppl6-exceptions.h"
#include "ppl6-sound.h"

namespace ppl6 {


CDDB::CDDB()
{
#ifdef HAVE_LIBCDDB
	conn=cddb_new();
	cddb_http_enable((cddb_conn_t *)conn);
	cddb_set_server_port((cddb_conn_t *)conn, 80);
	cddb_set_server_name((cddb_conn_t *)conn, "freedb.org");
	cddb_set_http_path_query((cddb_conn_t *)conn, "/~cddb/cddb.cgi");
	cddb_set_http_path_submit((cddb_conn_t *)conn, "/~cddb/submit.cgi");
#else
	conn=NULL;
#endif
}

CDDB::~CDDB()
{
#ifdef HAVE_LIBCDDB
	cddb_destroy((cddb_conn_t *)conn);
#endif
}

int CDDB::query(AudioCD &cd, std::list<Disc> &list)
{
#ifndef HAVE_LIBCDDB
	throw UnsupportedFeatureException("cddb");
#else
	list.clear();
	cddb_disc_t *disc = NULL;
	disc = cddb_disc_new();
	cddb_disc_set_length(disc,cd.totalAudioFrames()/75);

	for (size_t i=cd.firstTrack();i<=cd.lastTrack();i++) {
		AudioCD::Track track=cd.getTrack(i);
		cddb_track_t *t=cddb_track_new();
		cddb_track_set_frame_offset(t,track.start()+150);
		cddb_track_set_length(t,track.seconds());
		cddb_disc_add_track(disc,t);
	}

	int matches = cddb_query((cddb_conn_t *)conn, disc);
	if (matches == -1) {
		throw QueryFailed();
		//cddb_error_print(cddb_errno(conn));
	}
	while (matches > 0) {
		Disc d;
		getDisc(cddb_disc_get_discid(disc),cddb_disc_get_category_str(disc),d);
		list.push_back(d);
		matches--;
		if (matches > 0) {
			if (!cddb_query_next((cddb_conn_t *)conn, disc)) {
				fprintf(stderr, "query index out of bounds");
				throw QueryFailed();
			}
		}
	}
	return (int)list.size();
#endif
}

void CDDB::getDisc(unsigned int discId, const ppl6::CString &category, Disc &d)
{
#ifndef HAVE_LIBCDDB
	throw UnsupportedFeatureException("cddb");
#else
	cddb_disc_t *disc = cddb_disc_new();
	if (!disc) throw ppl6::OutOfMemoryException();
	cddb_disc_set_category_str(disc, (const char*)category);
	cddb_disc_set_discid(disc, discId);
	if (!cddb_read((cddb_conn_t *)conn, disc)) {
		cddb_error_print(cddb_errno((cddb_conn_t *)conn));
		throw QueryFailed();

	}
	storeDisc(d,disc);
	cddb_disc_destroy(disc);
#endif
}

void CDDB::storeDisc(Disc &d, void *disc)
{
#ifndef HAVE_LIBCDDB
	throw UnsupportedFeatureException("cddb");
#else
	d.discId=cddb_disc_get_discid((cddb_disc_t *)disc);
	d.category=cddb_disc_get_category_str((cddb_disc_t *)disc);
	d.genre=cddb_disc_get_genre((cddb_disc_t *)disc);
	d.length=cddb_disc_get_length((cddb_disc_t *)disc);
	d.year=cddb_disc_get_year((cddb_disc_t *)disc);
	d.Artist=cddb_disc_get_artist((cddb_disc_t *)disc);
	d.Title=cddb_disc_get_title((cddb_disc_t *)disc);
	d.Extra=cddb_disc_get_ext_data((cddb_disc_t *)disc);


	cddb_track_t *track = cddb_disc_get_track_first((cddb_disc_t *)disc);
	while (track != NULL) {
		addTrack(d,track);
	    track = cddb_disc_get_track_next((cddb_disc_t *)disc);
	}
#endif
}

void CDDB::addTrack(Disc &d, void *track)
{
#ifndef HAVE_LIBCDDB
	throw UnsupportedFeatureException("cddb");
#else
	Track t;
	t.number=cddb_track_get_number((cddb_track_t *)track);
	t.frame_offset=cddb_track_get_frame_offset((cddb_track_t *)track);
	t.length=cddb_track_get_length((cddb_track_t *)track);
	t.Artist=cddb_track_get_artist((cddb_track_t *)track);
	t.Title=cddb_track_get_title((cddb_track_t *)track);
	t.Extra=cddb_track_get_ext_data((cddb_track_t *)track);
	d.Tracks.push_back(t);
#endif
}

}	// EOF namespace ppl6
