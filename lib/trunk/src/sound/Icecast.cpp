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

#include "ppl7.h"
#include "ppl7-sound.h"

#ifdef HAVE_LIBSHOUT
#include <shout/shout.h>
#endif

namespace ppl7 {

#ifdef HAVE_LIBSHOUT
static Mutex		MutexInstance;
static ppluint32	instance=0;
#endif

/*!\class Icecast
 * \ingroup PPLGroupSound
 * \brief Audio-Daten zu einem Icecast-Server streamen
 *
 * \descr
 * Mit dieser Klasse kann ein MP3 oder OGG Datenstrom an einen Icecast-Server gestreamt werden.
 * Um sie verwenden zu können, muss auf dem System die Bibliothek "libshout" vorhanden
 * und beim "configure" gefunden worden sein.
 *
 * \see http://www.icecast.org/
 *
 * \example
 * Das folgende Beispiel zeigt einen einfachen Fall, wie eine MP3-Datei an einen
 * Icecast-Server gestreamt werden kann.
 * \include IcecastExample.cpp
 * \par
 * Nach Starten des Streams, sind folgende Informationen dem Icecast-Server zu entnehmen:
 * \image html icecast_status.png
 * \image latex icecast_status.png
 */

/*!\brief Konstruktor
 *
 * \descr
 * Der Konstruktor der Klasse prüft, ob bereits weitere Instanzen bestehen. Ist dies
 * nicht der Fall, wird die Shoutcast-Bibliothek initialisiert.
 *
 * \exception OutOfMemoryException: Wird geworfen, wenn kein Speicher mehr frei ist
 * \exception UnsupportedFeatureException: Wird geworfen, wenn libshout nicht einkompiliert ist.
 * \exception TooManyInstancesException: Es können maximal 65535 Instanzen der Klasse
 * angelegt werden
 */
Icecast::Icecast()
{
	shout=NULL;
	bconnected=false;
#ifdef HAVE_LIBSHOUT
	MutexInstance.lock();
	if (!instance) {
		shout_init();
	}
	if (instance>=65535) {
		MutexInstance.unlock();
		throw TooManyInstancesException("Icecast");
	}
	instance++;
	MutexInstance.unlock();
	shout=shout_new();
	if (!shout) {
		throw OutOfMemoryException();
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Destruktor
 *
 * \descr
 * Der Destruktor gibt den durch die Instanz der Klasse belegten Speicher wieder frei.
 * Falls es sich um die letzte Instanz der Klasse handelt, wird die Shoutcast-Bibliothek
 * zusätzlich ebenfalls deinitialisiert.
 */
Icecast::~Icecast()
{
#ifdef HAVE_LIBSHOUT
	if (bconnected) disconnect();
	if (shout) shout_free((shout_t*)shout);
	MutexInstance.lock();
	instance--;
	if (instance==0) {
		shout_shutdown();
	}
	MutexInstance.unlock();
#endif
}


/*!\brief Version der libshout auslesen
 *
 * \descr
 * Mit dieser Funktion kann die Version der einkompilierten "libshout" ausgelesen werden.
 * Die Funktion liefert zum einen einen String mit der Version zurück, zum anderen können
 * die einzelnen Bestandteile mittels der optionalen Parameter \p major, \p minor und \p patch
 * als Integer ausgelesen werden. Ein Wert, für den ein NULL-Pointer übergeben wurde, wird
 * nicht befüllt. Falls die einzelnen Bestandteile nicht benötigt werden, kann auch
 * die Funktion Icecast::version verwendet werden.
 *
 * @param major Pointer auf einen Integer, der die Major-Version aufnehmen soll
 * @param minor Pointer auf einen Integer, der die Minor-Version aufnehmen soll
 * @param patch Pointer auf einen Integer, der den Patch-Level der Version aufnehmen soll
 *
 * @return String mit der Version
 */
String Icecast::getVersion(int *major, int *minor, int *patch) const
{
#ifdef HAVE_LIBSHOUT
	return shout_version(major,minor,patch);
#else
	return "";
#endif
}

/*!\brief Version der libshout auslesen
 *
 * \descr
 * Mit dieser Funktion kann die Version der einkompilierten "libshout" als String ausgelesen werden.
 *
 * @return String mit der Version der libshout.
 */
String Icecast::version() const
{
#ifdef HAVE_LIBSHOUT
	return shout_version(NULL,NULL,NULL);
#else
	return "";
#endif
}

/*!\brief Verbindung zum Server vorhanden?
 *
 * \descr
 * Diese Funktion liefert \c true zurück, wenn eine Verbindung zum Server vorhanden ist,
 * andernfalls \c false.
 *
 * @return Gibt \c true oder \c false zurück.
 */
bool Icecast::connected()
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_get_connected((shout_t*)shout);
	if (ret==SHOUTERR_CONNECTED) {
		bconnected=true;
		return true;
	}
#endif
	bconnected=false;
	return false;
}

/*!\brief Verbindungsdaten festlegen
 *
 * \descr
 * Mit dieser Funktion werden die wichtigsten Daten für die Verbindung zum Server
 * festgelegt.
 *
 * @param host String mit dem Hostnamen oder der IP-Adresse des Icecast-Servers
 * @param port Integer mit der Port-Nummer des Servers
 * @param password String mit dem Passwort zur Authentisierung
 *
 * \note Üblicherweise wird als Benutzername zur Authentifizierung "source" verwendet.
 * Soll ein anderer Benutzername verwendet werden, kann dieser mit der Funktion
 * Icecast::setUser festgelegt werden.
 *
 */
void Icecast::setConnection(const String &host, int port, const String &password)
{
#ifdef HAVE_LIBSHOUT
	int ret;
	ret=shout_set_host((shout_t*)shout,host.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_host");
	}
	ret=shout_set_port((shout_t*)shout,port);
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_port");
	}
	ret=shout_set_password((shout_t*)shout,password.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_password");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Hostname des Servers auslesen
 *
 * \descr
 * Den Hostnamen des Icecast-Servers auslesen.
 *
 * @return String mit dem Hostnamen des Servers.
 */
String Icecast::host() const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_host((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Port des Servers auslesen
 *
 * \descr
 * Den Port des Icecast-Servers auslesen.
 *
 * @return Integer mit dem Port des Servers.
 */
int Icecast::port() const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_port((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Passwort für die Authentisierung auslesen
 *
 * \descr
 * Das für die Authentisierung am Server verwendete Passwort auslesen.
 *
 * @return String mit dem Passwort.
 */
String Icecast::password() const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_password((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}


/*!\brief Mount-Punkt festlegen
 *
 * \descr
 * Mit dieser Funktion wird der Mount-Punkt des Streams am Server festgelegt.
 *
 * @param mount String mit dem Namen des Mount-Punkts.
 *
 * \example
 * Im Icecast-Server ist folgender Mount-Punkt definiert:
 \verbatim
    <mount>
        <mount-name>/live</mount-name>
        <username>source</username>
        <password>icesource</password>
    </mount>
 \endverbatim
 * Der Name des Mount-Punkts ist also "/live". Die Funktion würde also folgendermassen
 * aufgerufen werden:
 * \code
 * ppl6::Icecast ice;
 * ice.setConnection("icecast.example.org",1234,"icesource");
 * ice.setMount("/live");
 * \endcode
 */
void Icecast::setMount(const String &mount)
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_mount((shout_t*)shout,mount.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_mount");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Mount-Punkt auslesen
 *
 * \descr
 * Den für diesen Stream definierten Mount-Punkt auslesen.
 *
 * @return String mit dem Namen des Mount-Punkts.
 */
String Icecast::mount() const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_mount((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

	
/*!\brief Name des Streams festlegen
 *
 * \descr
 * Den Namen des Streams festlegen
 *
 * @param name String mit dem Namen des Streams
 */
void Icecast::setName(const String &name)
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_name((shout_t*)shout,name.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_name");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Name des Streams auslesen
 *
 * \desc
 * Den Namen des Streams auslesen.
 *
 * @return String mit dem Namen des Streams
 *
 *
 */
String Icecast::name() const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_name((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

	
/*!\brief URL der Webseite festlegen
 *
 * \descr
 * Mit dieser Funktion wird die URL der Webseite definiert, unter der man weitere Informationen zum Stream finden kann.
 *
 * @param url String mit der URL der Webseite
 */
void Icecast::setUrl(const String &url)
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_url((shout_t*)shout,url.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_url");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief URL auslesen
 *
 * \descr
 * Auslesen der zur Zeit definierten URL.
 *
 * @return String mit der URL
 * \see Icecast::setURL - Setzen der URL
 */
String Icecast::url() const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_url((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

	
/*!\brief Genre festlegen
 *
 * \descr
 * Sets the genre (or genres) of the stream. This is usually a keyword list, eg "pop rock rap".
 *
 * @param genre String mit den Genres
 */
void Icecast::setGenre(const String &genre)
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_genre((shout_t*)shout,genre.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_genre");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Genres auslesen
 *
 * \descr
 * Gibt die zur Zeit definierten Genres als String zurück
 *
 * @return String mir den Genres
 */
String Icecast::genre() const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_genre((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Benutzernamen für die Authentifizierung festlegen
 *
 * \descr
 * Sets the user to authenticate as, for protocols that can use this parameter. The default is "source".
 *
 * @param user String mit dem Benutzernamen
 */
void Icecast::setUser(const String &user)
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_user((shout_t*)shout,user.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_user");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Benutzernamen auslesen
 *
 * \descr
 * Mit dieser Funktion wird der zur Authentifizierung verwendete Benutzername ausgelesen.
 *
 * @return String mit dem Benutzernamen
 */
String Icecast::user() const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_user((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}


/*!\brief User Agent setzen
 *
 * \descr
 * Sets the user agent header. This is "libshout/VERSION" by default. If you don't know what this function is for, don't use it.
 *
 * @param agent String mit dem Namen des Agent
 */
void Icecast::setAgent(const String &agent)
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_agent((shout_t*)shout,agent.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_agent");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Agent auslesen
 *
 * \descr
 * Den Namen des zur Zeit gesetzten Agent auslesen.
 *
 * @return String mit dem Namen des Agent
 */
String Icecast::agent() const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_agent((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}


/*!\brief Beschreibung des Streams setzen
 *
 * \descr
 * Setzt die Beschreibung für diesen Stream.
 *
 * @param description String mit der Beschreibung.
 */
void Icecast::setDescription(const String &description)
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_description((shout_t*)shout,description.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_description");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Beschreibung auslesen
 *
 * \descr
 * Beschreibung für diesen Stream auslesen.
 *
 * @return String mit der Beschreibung
 */
String Icecast::description() const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_description((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}



/*!\brief Stream auf dem Server archivieren
 *
 * \descr
 * If the server supports it, you can request that your stream be archived
 * on the server under the name \p file. This can quickly eat a lot of disk
 * space, so think twice before setting it.
 *
 * @param file Name des Archiv-Files
 */
void Icecast::setDumpfile(const String &file)
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_dumpfile((shout_t*)shout,file.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_dumpfile");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Name des Archiv-Files auslesen
 *
 * \descr
 * Mit dieser Funktion wird der Name des Archiv-Files auf dem Server ausgelesen, sofern vorher
 * eins mit der Funktion Icecast::setDumpfile gesetzt wurde.
 *
 * @return String mit dem Namen des Archiv-Files.
 */
String Icecast::dumpfile() const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_dumpfile((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Audio-Information setzten
 *
 * \descr
 * Sets a stream audio parameter (eg bitrate, samplerate, channels or quality).
 * The currently defined parameters are listed in the Audio Info Constants section,
 * but you are free to add additional fields if your directory server understands them.
 *
 * @param name Name des Parameters
 * @param value Inhalt des Parameters
 */
void Icecast::setAudioInfo(const String &name, const String &value)
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_audio_info((shout_t*)shout,name.toUtf8(),value.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_audio_info");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}

/*!\brief Audio-Information auslesen
 *
 * \descr
 * Gibt die unter dem Namen \p name gespeicherten Audio-Informationen zurück.
 *
 * @param name Name des Parameters
 * @return String mit dem Wert des Parameters
 */
String Icecast::audioInfo(const String &name) const
{
#ifdef HAVE_LIBSHOUT
	return shout_get_audio_info((shout_t*)shout,name.toUtf8());
#else
	throw UnsupportedFeatureException("Icecast");
#endif
}
/*!\brief Stream als öffentlich bekannt geben
 *
 * \descr
 * Setting this to \c true asks the server to list the stream in any
 * directories it knows about. To suppress listing, set this to \c false.
 * The default is \c false.
 *
 * @param makepublic
 */
void Icecast::setPublic(bool makepublic)
{
#ifdef HAVE_LIBSHOUT
	int param=0;
	if (makepublic) param=1;
	int ret=shout_set_public((shout_t*)shout,param);
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_public");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif

}

/*!\brief Stream-Format auf MP3 festlegen
 *
 * \descr
 * Mit dieser Funktion wird das Datenformat des Streams auf MP3 festgelegt.
 */
void Icecast::setFormatMP3()
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_format((shout_t*)shout,SHOUT_FORMAT_MP3);
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_format");
	}
	ret=shout_set_protocol((shout_t*)shout,SHOUT_PROTOCOL_HTTP);
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_protocol");
	}

#else
	throw UnsupportedFeatureException("Icecast");
#endif

}

/*!\brief Stream-Format auf OGG festlegen
 *
 * \descr
 * Mit dieser Funktion wird das Datenformat des Streams auf OGG festgelegt.
 */
void Icecast::setFormatOGG()
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_format((shout_t*)shout,SHOUT_FORMAT_OGG);
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_format");
	}
	ret=shout_set_protocol((shout_t*)shout,SHOUT_PROTOCOL_HTTP);
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_protocol");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif

}

/*!\brief Verbindung auf Non-Blocking setzen
 *
 * \descr
 * Durch Aufruf dieser Funktion wird der Stream auf Non-Blocking eingestellt.
 * Dies muss vor Verbindungsaufbau geschehen (Icecast::connect) und kann nicht
 * danach nicht verändert werden.
 *
 * @param flag
 */
void Icecast::setNonBlocking(bool flag)
{
#ifdef HAVE_LIBSHOUT
	int ret=shout_set_nonblocking((shout_t*)shout,flag);
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_set_nonblocking");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif

}

/*!\brief Verbindung zum Server herstellen
 *
 * \descr
 * Mit dieser Funktion wird die Verbindung zum Icecast-Server hergestellt.
 *
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception IllegalArgumentException: Es fehlen Parameter, z.B. wenn Icecast::setConnection nicht aufgerufen wurde
 * \exception AlreadyConnectedException: Es besteht bereits eine Verbindung
 * \exception ConnectionFailedException: Verbindung fehlgeschlagen
 * \exception SocketException: Bei der Kommunikation mit dem Server ist ein Fehler aufgetreten
 * \exception LoginRefusedException: Anmeldung fehlgeschlagen (z.B. falsches Passwort)
 * \exception NoConnectionException: Es besteht keine Verbindung
 * \exception OperationFailedException: Verbindung aus anderen Gründen fehlgeschlagen
 */
void Icecast::connect()
{
#ifdef HAVE_LIBSHOUT
	if (bconnected) disconnect();
	int ret=shout_open((shout_t*)shout);
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		else if (ret==SHOUTERR_INSANE) throw IllegalArgumentException("Check if host, port, and password are set");
		else if (ret==SHOUTERR_CONNECTED) throw AlreadyConnectedException();
		else if (ret==SHOUTERR_NOCONNECT) throw ConnectionFailedException();
		else if (ret==SHOUTERR_UNSUPPORTED) throw OperationFailedException("The protocol/format combination is unsupported");
		else if (ret==SHOUTERR_SOCKET) throw SocketException();
		else if (ret==SHOUTERR_NOLOGIN) throw LoginRefusedException();
		else if (ret==SHOUTERR_UNCONNECTED) throw NoConnectionException();
		throw OperationFailedException("shout_open");
	}
	bconnected=true;
#else
	throw UnsupportedFeatureException("Icecast");
#endif

}

/*!\brief Verbindung trennen
 *
 * \descr
 * Durch Aufruf dieser Funktion wird eine bestehende Verbindung zum Server getrennt.
 *
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception IllegalArgumentException: Es fehlen Parameter
 * \exception SocketException: Bei der Kommunikation mit dem Server ist ein Fehler aufgetreten
 * \exception LoginRefusedException: Anmeldung fehlgeschlagen (z.B. falsches Passwort)
 * \exception NoConnectionException: Es besteht keine Verbindung
 * \exception OperationFailedException: Verbindung aus anderen Gründen fehlgeschlagen
 *
 */
void Icecast::disconnect()
{
#ifdef HAVE_LIBSHOUT
	if (!bconnected) return;
	int ret=shout_close((shout_t*)shout);
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		else if (ret==SHOUTERR_INSANE) throw IllegalArgumentException("Check if host, port, and password are set");
		else if (ret==SHOUTERR_NOCONNECT) {
			bconnected=false;
			return;
		}
		else if (ret==SHOUTERR_UNSUPPORTED) throw OperationFailedException("The protocol/format combination is unsupported");
		else if (ret==SHOUTERR_SOCKET) throw SocketException();
		else if (ret==SHOUTERR_NOLOGIN) throw LoginRefusedException();
		else if (ret==SHOUTERR_UNCONNECTED) throw NoConnectionException();
		throw OperationFailedException("shout_close");
	}
	bconnected=false;
#else
	throw UnsupportedFeatureException("Icecast");
#endif

}

/*!\brief Audiodaten senden
 *
 * \descr
 * Mit dieser Funktion werden die bereits komprimierten Audio-Daten an den Icecast-
 * Server gesendet.
 *
 * @param buffer Pointer auf den Beginn der zu sendenden Audiodaten
 * @param bytes Anzahl zu sendender Bytes
 *
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception IllegalArgumentException: Es fehlen Parameter
 * \exception SocketException: Bei der Kommunikation mit dem Server ist ein Fehler aufgetreten
 * \exception LoginRefusedException: Anmeldung fehlgeschlagen (z.B. falsches Passwort)
 * \exception NoConnectionException: Es besteht keine Verbindung
 * \exception OperationFailedException: Übertragung aus anderen Gründen fehlgeschlagen
 */
void Icecast::send(const void *buffer, size_t bytes)
{
#ifdef HAVE_LIBSHOUT
	if (!bconnected) throw NoConnectionException();
	int ret=shout_send((shout_t*)shout,(const unsigned char*)buffer,bytes);
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		else if (ret==SHOUTERR_INSANE) throw IllegalArgumentException("Check if host, port, and password are set");
		else if (ret==SHOUTERR_UNSUPPORTED) throw OperationFailedException("The protocol/format combination is unsupported");
		else if (ret==SHOUTERR_SOCKET) throw SocketException();
		else if (ret==SHOUTERR_NOLOGIN) throw LoginRefusedException();
		else if (ret==SHOUTERR_UNCONNECTED) throw NoConnectionException();
		throw OperationFailedException("shout_send");
	}
#else
	throw UnsupportedFeatureException("Icecast");
#endif

}

/*!\brief Number of milliseconds the caller should wait
 *
 * \descr
 * Returns the number of milliseconds the caller should wait before calling
 * Icecast::send again. This function is provided as an alternative to
 * Icecast::sync for applications that may wish to do other processing in the meantime.
 *
 * @return Number of milliseconds the caller should wait.
 */
int Icecast::delay()
{
#ifdef HAVE_LIBSHOUT
	if (!bconnected) return 0;
	return shout_delay((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif

}

/*!\brief Synchronise with Stream
 *
 * \descr
 * Causes the caller to sleep for the amount of time necessary to play back audio
 * sent since the last call to shout_sync. Should be called before every call to
 * Icecast::send to ensure that audio data is sent to the server at the correct speed.
 * Alternatively, the caller may use Icecast::delay to determine the number of
 * milliseconds to wait and delay itself.
 */
void Icecast::sync()
{
#ifdef HAVE_LIBSHOUT
	if (!bconnected) return;
	shout_sync((shout_t*)shout);
#else
	throw UnsupportedFeatureException("Icecast");
#endif

}

/*!\brief Metadaten setzen
 *
 * \descr
 * Sets metadata on the connection. Only MP3 streams support this type of metadata update.
 * You may use this function on defined but closed connections
 * (this is useful if you simply want to set the metadata for a stream provided by
 * another process).
 *
 * @param name Name der Metadaten
 * @param value Inhalt der Metadaten
 */
void Icecast::sendMetadata(const String &name, const String &value)
{
#ifdef HAVE_LIBSHOUT
	if (!bconnected) return;
	shout_metadata_t *meta=shout_metadata_new();
	int ret;
	ret=shout_metadata_add(meta,name.toUtf8(),value.toUtf8());
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		throw OperationFailedException("shout_metadata_add");
	}
	ret=shout_set_metadata((shout_t*)shout,meta);
	if(ret!=SHOUTERR_SUCCESS) {
		if (ret==SHOUTERR_MALLOC) throw OutOfMemoryException();
		else if (ret==SHOUTERR_INSANE) throw IllegalArgumentException("Check if host, port, and password are set");
		else if (ret==SHOUTERR_NOCONNECT) throw ConnectionFailedException();
		else if (ret==SHOUTERR_UNSUPPORTED) throw OperationFailedException("The protocol/format combination is unsupported");
		else if (ret==SHOUTERR_SOCKET) throw SocketException();
		else if (ret==SHOUTERR_NOLOGIN) throw LoginRefusedException();
		else if (ret==SHOUTERR_UNCONNECTED) throw NoConnectionException();
		throw OperationFailedException("shout_set_metadata");
	}
	shout_metadata_free(meta);
#else
	throw UnsupportedFeatureException("Icecast");
#endif

}

/*!\brief Aktuellen Titel übermitteln
 *
 * \desc
 * Mit dieser Funktion wird der aktuelle Titel (bzw. der angegebene String \p title) als
 * Metdataten vom Typ "song" an den Icecast-Server übermittelt.
 *
 * @param title String mit dem Namen des aktuellen Titels
 */
void Icecast::setTitle(const String &title)
{
#ifdef HAVE_LIBSHOUT
	sendMetadata("song",title);
#else
	throw UnsupportedFeatureException("Icecast");
#endif

}


}	// EOF namespace ppl6
