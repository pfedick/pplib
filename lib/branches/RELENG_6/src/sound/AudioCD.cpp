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

#ifdef HAVE_LIBCDIO
#include <cdio/cdio.h>
#include <cdio/cdda.h>
#include <cdio/paranoia.h>
#include <cdio/cd_types.h>
#endif

#include "ppl6.h"
#include "ppl6-exceptions.h"
#include "ppl6-sound.h"

namespace ppl6 {

AudioCD::AudioCD()
{
	cdio=NULL;
	drive=NULL;
	num_audio_tracks=0;
	i_tracks=0;
	first_track_num=0;
	audio_frames=0;
}

AudioCD::~AudioCD()
{
	closeDevice();
}

void AudioCD::closeDevice()
{
#ifndef HAVE_LIBCDIO
	throw UnsupportedFeatureException("cdio");
#else

	if (drive) cdio_cddap_close_no_free_cdio((cdrom_drive_t*)drive);
	if (cdio) cdio_destroy((CdIo_t *)cdio);
	drive=NULL;
	cdio=NULL;
	num_audio_tracks=0;
	i_tracks=0;
	first_track_num=0;
#endif
}

void AudioCD::openDevice(const ppl6::CString &device)
{
#ifndef HAVE_LIBCDIO
	throw UnsupportedFeatureException("cdio");
#else
	if (cdio) closeDevice();

	cdio=NULL;
	myDevice=device;
	if (myDevice.IsEmpty()) {
		CdIo_t *p_cdio = cdio_open (NULL, DRIVER_DEVICE);
		if (NULL != p_cdio) {
			myDevice=cdio_get_default_device(p_cdio);
			cdio_destroy(p_cdio);
		}
	}
	cdio=cdio_open((const char*)myDevice, DRIVER_DEVICE);
	if (!cdio) throw DeviceOpenFailed("%s",(const char*)myDevice);

	drive=cdio_cddap_identify_cdio((CdIo_t *)cdio, 0, NULL);
	if (!drive) {
		closeDevice();
		throw DeviceOpenFailed("%s",(const char*)myDevice);
	}
	if ( 0 != cdio_cddap_open((cdrom_drive_t*)drive) ) {
		throw DeviceOpenFailed("%s",(const char*)myDevice);
	}

	first_track_num = (size_t)cdio_get_first_track_num((CdIo_t *)cdio);
	i_tracks = (size_t)cdio_get_num_tracks((CdIo_t *)cdio);
	countAudioTracks();
#endif
}

void AudioCD::countAudioTracks()
{
#ifndef HAVE_LIBCDIO
	throw UnsupportedFeatureException("cdio");
#else
	num_audio_tracks=0;
	audio_frames=0;
	for (size_t i = first_track_num; i <= i_tracks; i++) {
		if (TRACK_FORMAT_AUDIO == cdio_get_track_format((CdIo_t *)cdio, i)) {
			num_audio_tracks++;
			audio_frames+= cdio_get_track_last_lsn((CdIo_t *)cdio, i) -
					cdio_get_track_lsn((CdIo_t *)cdio, i)+1;

		}
	}
#endif
}

size_t AudioCD::totalAudioFrames() const
{
	return audio_frames;
}


const ppl6::CString &AudioCD::deviceName() const
{
	return myDevice;
}

size_t AudioCD::firstTrack() const
{
	return first_track_num;
}

size_t AudioCD::lastTrack() const
{
	return first_track_num+i_tracks-1;
}

size_t AudioCD::numTotalTracks() const
{
	return i_tracks;
}

size_t AudioCD::numAudioTracks() const
{
	return num_audio_tracks;
}

bool AudioCD::dataIsBigEndian() const
{
#ifndef HAVE_LIBCDIO
	throw UnsupportedFeatureException("cdio");
#else
	if (drive==NULL) throw AudioCD::DeviceNotOpen();
	return data_bigendianp((cdrom_drive_t*)drive);
#endif
}

bool AudioCD::dataIsLittleEndian() const
{
#ifndef HAVE_LIBCDIO
	throw UnsupportedFeatureException("cdio");
#else
	if (drive==NULL) throw AudioCD::DeviceNotOpen();
	if (data_bigendianp((cdrom_drive_t*)drive)) return false;
	return true;
#endif
}

AudioCD::Track AudioCD::getTrack(int track)
{
#ifndef HAVE_LIBCDIO
	throw UnsupportedFeatureException("cdio");
#else
	if (!drive) throw DeviceNotOpen();
	if (cdio_cddap_track_audiop((cdrom_drive_t*)drive, track)!=1) throw InvalidAudioTrack("%i",track);
	AudioCD::Track t;
	t._track=track;
	t._start=(size_t)cdio_cddap_track_firstsector((cdrom_drive_t*)drive, track);
	t._end=(size_t)cdio_cddap_track_lastsector((cdrom_drive_t*)drive, track);
	t._channels=cdio_cddap_track_channels((cdrom_drive_t*)drive,track);
	t._hasPreemphasis=cdio_cddap_track_preemp((cdrom_drive_t*)drive,track);
	t._hasCopyPermit=cdio_cddap_track_audiop((cdrom_drive_t*)drive,track);
	return t;
#endif
}

bool AudioCD::isAudioTrack(int track)
{
#ifndef HAVE_LIBCDIO
	throw UnsupportedFeatureException("cdio");
#else
	if (!drive) throw DeviceNotOpen();
	if (cdio_cddap_track_audiop((cdrom_drive_t*)drive, track)==1) return true;
	return false;
#endif
}


AudioCD::Track::Track()
{
	_track=0;
	_start=0;
	_end=0;
	_hasPreemphasis=false;
	_hasCopyPermit=false;
	_channels=0;
}

int AudioCD::Track::track() const
{
	return _track;
}

size_t AudioCD::Track::start() const
{
	return _start;
}

size_t AudioCD::Track::end() const
{
	return _end;
}

size_t AudioCD::Track::size() const
{
	return _end-_start+1;
}

size_t AudioCD::Track::seconds() const
{
	return round((float)(_end-_start+1)/75.0f);
}

bool AudioCD::Track::hasPreemphasis() const
{
	return _hasPreemphasis;
}

bool AudioCD::Track::hasCopyPermit() const
{
	return _hasCopyPermit;
}

int AudioCD::Track::channels() const
{
	return _channels;
}


}	// EOF namespace ppl6
