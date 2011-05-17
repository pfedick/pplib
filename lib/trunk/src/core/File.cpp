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

//#define WIN32FILES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_WCHAR_H
	#include <wchar.h>
#endif
#include <time.h>
#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif

#ifdef HAVE_SYS_MMAN_H
	#include <sys/mman.h>
#endif

#ifdef HAVE_FCNTL_H
	#include <fcntl.h>
#endif
#ifdef HAVE_SYS_TYPES_H
	#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
	#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_FILE_H
	#include <sys/file.h>
#endif
#ifdef HAVE_STDARG_H
	#include <stdarg.h>
#endif
#ifdef HAVE_ERRNO_H
	#include <errno.h>
#endif
#ifdef _WIN32
#include <io.h>
#define WIN32_LEAN_AND_MEAN		// Keine MFCs
#define popen _popen
#define pclose _pclose

#include <windows.h>
#ifdef DeleteFile
#undef DeleteFile
#endif

#ifdef CopyFile
#undef CopyFile
#endif


#ifdef MoveFile
#undef MoveFile
#endif

#endif
#include "ppl7.h"

namespace ppl7 {

#define COPYBYTES_BUFFERSIZE 1024*1024

/*!\class File
 * \ingroup PPLGroupFileIO
 * \brief Dateizugriff
 *
 * \header \#include <ppl7.h>
 * \desc
 * Mit dieser Klasse können Dateien geladen, verändert und gespeichert werden.
 * Sie dient als Wrapper-Klasse für die Low-Level Funktionen des Betriebssystems.
 *
 *
 * \section ppl7_File_Filemodi Dateimodi
 * Die File-Klasse unterstützt folgende Dateimodi:
 * <ul>
<li><b>r</b><br>Open text file for reading.  The stream is positioned at the beginning of the file.
</li>
<li><b>r+</b><br>Open for reading and writing.  The stream is positioned at the beginning of the file.
</li>
<li><b>w</b><br>Truncate  file to zero length or create text file for writing.  The stream is positioned at the beginning
of the file.
</li>
<li><b>w+</b><br>
Open for reading and writing.  The file is created if it does not exist, otherwise it is truncated. The
stream is positioned at the beginning of the file.
</li>
<li><b>a</b><br>Open  for  appending  (writing at end of file).  The file is created if it does not exist.
The stream is positioned at the end of the file.
</li>
<li><b>a+</b><br>Open for reading and appending (writing at end of file).  The file is
created if it does not exist.   The  initial  file  position for reading is at the beginning of
the file, but output is always appended to the end of the file.
</li>
<li><b>b</b> (Optionaler Zusatzparameter)<br>
The mode string can also include the letter ‘‘b’’ either as a last character or as a character between the char-
       acters in any of the two-character strings described above.  This is strictly for compatibility with C89 and has
       no effect; the ‘‘b’’ is ignored on all POSIX conforming systems, including Linux.  (Other systems may treat text
       files  and  binary files differently, and adding the ‘‘b’’ may be a good idea if you do I/O to a binary file and
       expect that your program may be ported to non-Unix environments.)
       </li>
</ul>
 *
 */


/*!\enum File::FileMode
 * \brief Zugriffsmodus beim Öffnen einer Datei
 *
 * \desc
 * Zugriffsmodus beim Öffnen einer Datei mit File::open
 *
 */

/*!\var File::FileMode File::READ
 * \brief Datei zum Lesen öffnen, Zeiger wird an den Anfang der Datei gesetzt
 */

/*!\var File::FileMode File::WRITE
 * \brief Datei zum Schreiben öffnen. Falls die Datei schon vorhanden ist, wird sie gelöscht.
 */

/*!\var File::FileMode File::READWRITE
 * \brief Datei zum Lesen und Schreiben öffnen. Zeiger wird an den Anfang der Datei positioniert.
 */

/*!\var File::FileMode File::APPEND
 * \brief Datei zum Schreiben öffnen, Zeiger wird ans Ende der Datei positioniert.
 */


File::File ()
//! \brief Konstruktor der Klasse
{
	#ifdef USEWIN32
		hf=0;
	#endif
	MapBase=NULL;
	ff=NULL;
	mysize=pos=0;
	buffer=NULL;
	LastMapStart=LastMapSize=0;
	LastMapProtection=0;
	ReadAhead=0;
	isPopen=false;
}

File::File (const String &filename, FileMode mode)
/*!\brief Konstruktor der Klasse mit gleichzeitigem Öffnen einer Datei
 *
 * Konstruktor der Klasse, mit dem gleichzeitig eine Datei geöffnet wird.
 * @param[in] filename Name der zu öffnenden Datei
 * @param[in] mode Zugriffsmodus. Defaultmäßig wird die Datei zum binären Lesen
 * geöffnet (siehe \ref ppl7_File_Filemodi)
 */
{
	MapBase=NULL;
	#ifdef USEWIN32
		hf=0;
	#endif
	ff=NULL;
	mysize=pos=0;
	buffer=NULL;
	LastMapStart=LastMapSize=0;
	LastMapProtection=0;
	ReadAhead=0;
	isPopen=false;
	open (filename,mode);
}


File::File (FILE * handle)
//! \brief Konstruktor der Klasse mit Übernahme eines C-Filehandles einer bereits geöffneten Datei
{
	MapBase=NULL;
	ff=NULL;
	buffer=NULL;
	mysize=pos=0;
	LastMapStart=LastMapSize=0;
	LastMapProtection=0;
	ReadAhead=0;
	isPopen=false;
	if (handle!=NULL) {
		if (ff!=NULL) close();
		ff=handle;
		mysize=size();
		seek(0);
	}
}

File::~File()
//! \brief Destruktor der Klasse
{
	if (ff!=NULL) {
		close();
	}
}

const char *File::fmode(FileMode mode)
{
	switch (mode) {
		case READ: return "rb";
		case WRITE: return "wb";
		case READWRITE: return "r+b";
		case APPEND: return "ab";
		default:
			throw IllegalFilemodeException();
	}
}

/*!\brief Exception anhand errno-Variable werfen
 *
 * \desc
 * Diese Funktion wird intern verwendet, um nach Auftreten eines Fehlers, anhand der globalen
 * "errno"-Variablen die passende Exception zu werfen.
 *
 * @param filename Dateiname, bei der der Fehler aufgetreten ist
 */
void File::throwErrno(const String &filename)
{
	switch (errno) {
		case ENOMEM: throw OutOfMemoryException();
		case EINVAL: throw InvalidArgumentsException();
		case ENOTDIR:
		case ENAMETOOLONG: throw InvalidFileNameException(filename);
		case EACCES:
		case EPERM: throw PermissionDeniedException(filename);
		case ENOENT: throw FileNotFoundException(filename);
		case ELOOP: throw TooManySymbolicLinksException(filename);
		case EISDIR: throw NoRegularFileException(filename);
		case EROFS: throw ReadOnlyException(filename);
		case EMFILE: throw TooManyOpenFilesException();
		case EOPNOTSUPP: throw UnsupportedFileOperationException(filename);
		case ENOSPC: throw FilesystemFullException();
		case EDQUOT: throw QuotaExceededException();
		case EIO: throw IOErrorException();
		case EBADF: throw BadFiledescriptorException();
		case EFAULT: throw BadAddressException();
		case EOVERFLOW: throw OverflowException();
		case EEXIST: throw FileExistsException();
		case EAGAIN: throw OperationBlockedException();
		case EDEADLK: throw DeadlockException();
		case EINTR: throw OperationInterruptedException();
		case ENOLCK: throw TooManyLocksException();
		case ESPIPE: throw IllegalOperationOnPipeException();

		/*

		 *
		 */

		default: throw FileOpenException(filename);
	}
}

void File::throwErrno()
{
	throwErrno(filename());
}

/*!\brief Datei öffnen
 *
 * Mit dieser Funktion wird eine Datei zum Lesen, Schreiben oder beides geöffnet.
 * @param[in] filename Dateiname
 * @param mode Zugriffsmodus
 * @return Bei Erfolg liefert die Funktion True (1) zurück, sonst false (0).
 */
void File::open (const String &filename, FileMode mode)
{
	close();
	// fopen stuerzt ab, wenn filename leer ist
	if (filename.isEmpty()) throw IllegalArgumentException();
	if ((ff=(FILE*)::fopen((const char*)filename.toLocalEncoding(),fmode(mode)))==NULL) {
		throwErrno(filename);
	}
	mysize=size();
	seek(0);
	setFilename(filename);
}

/*!\brief Datei zum Lesen oder Schreiben öffnen
 *
 * Mit dieser Funktion wird eine Datei zum Lesen, Schreiben oder beides geöffnet.
 *
 * \param filename Dateiname
 * \param mode String, der angibt, wie die Datei geöffnet werden soll (siehe \ref ppl7_File_Filemodi)
 *
 * \return Bei Erfolg liefert die Funktion True (1) zurück, sonst false (0).
 */
void File::open (const char * filename, FileMode mode, ...)
{
	if (!filename) throw IllegalArgumentException();
	String str;
	va_list args;
	va_start(args, mode);
	str.vasprintf(filename,args);
	open(str,mode);
}

/*!\brief Eine temporäre Datei zum Lesen und Schreiben öffnen
 *
 * Diese Funktion erzeugt eine temporäre Datei mit einem eindeutigen Namen.
 * Dieser Name wird aus \p filetemplate erzeugt. Dazu  müssen  die letzten
 * sechs  Buchstaben  des  Parameters template XXXXXX sein, diese werden dann
 * durch eine Zeichenkette ersetzt, die diesen Dateinamen eindeutig  macht.
 * Die  Datei  wird dann mit dem Modus read/write und den Rechten 0666 erzeugt.
 *
 * @param[in] filetemplate Pfad und Vorlage für den zu erstellenden Dateinamen
 * @param[in] ... Optionale Parameter
 */
void File::openTemp(const char *filetemplate, ...)
{
	#ifdef HAVE_MKSTEMP
		close();
		String t;
		va_list args;
		va_start(args, filetemplate);
		t.vasprintf(filetemplate,args);
		va_end(args);

		int f=::mkstemp((char*)((const char*)t.toLocalEncoding()));
		if (f<0) throwErrno(t);
		FILE *ff=::fdopen(f, "r+b");
		if (!ff) throwErrno(t);
		try {
			open(ff);
		} catch (...) {
			try {
				::fclose(ff);
				::close(f);
			} catch (...) {

			}
			throw;
		}
		setFilename(t);
	#else
		throw UnsupportedFeatureException("ppl7::File::openTemp, no mkstemp available");
	#endif
}

/*!\brief Datei schließen
 *
 * \desc
 * Diese Funktion schließt die aktuell geöffnete Datei. Sie wird automatisch vom Destruktor der
 * Klasse aufgerufen, so dass ihr expliziter Aufruf nicht erforderlich ist.
 * \par
 * Wenn  der  Stream  zur  Ausgabe  eingerichtet  war,  werden  gepufferte  Daten  zuerst  durch FileObject::Flush
 * geschrieben. Der zugeordnete Datei-Deskriptor wird geschlossen.
 *
 */
void File::close()
{
	if (MapBase!=NULL) {
		this->munmap(MapBase, (size_t)LastMapSize);
	}

	setFilename(L"");
	if (ff!=NULL) {
		int ret=1;
		if (buffer!=0) {
			free (buffer);
			buffer=0;
		}
		#ifdef WIN32FILES
			CloseHandle((HANDLE)ff);
		#else
			if(isPopen) {
				if (::pclose ((FILE*)ff)!=0) ret=0;
			} else {
				if (::fclose ((FILE*)ff)!=0) ret=0;
			}
		#endif
		isPopen=false;
		ff=NULL;
		mysize=0;
		pos=0;
		if (ret==0) throwErrno(filename());
		return;
	}
	//throw FileNotOpenException();
}

/*!\brief Prüfen, ob eine Datei geöffnet ist
 *
 * \header \#include <ppl7.h>
 * \desc
 * Mit dieser Funktion kann geprüft werden, ob die mit File assoziierte Datei
 * gerade geöffnet ist.
 * \return Die Funktion liefert \p true zurück, wenn die Datei offen ist, ansonsten \p false.
 */
bool File::isOpen() const
{
	if (ff!=NULL) return true;
	return false;
}


/*!\brief Größe der geöffneten Datei
 *
 * Diese Funktion liefert die Größe der geöffneten Datei in Bytes zurück.
 * \return Größe der Datei in Bytes, oder -1, wenn ein Fehler aufgetreten ist.
 *
 * \note Die Funktion ist File::Lof vorzuziehen
 */
ppluint64 File::size () const
{
	if (ff!=NULL) {
		#ifdef WIN32FILES
			BY_HANDLE_FILE_INFORMATION info;
			if (GetFileInformationByHandle((HANDLE)ff,&info))
				return (((ppluint64)info.nFileSizeHigh)<<32)+info.nFileSizeLow;
			throwErrno(filename());	// ???
		#else
			struct stat buf;
			if ((::fstat (fileno((FILE*)ff),&buf))==0)
				return ((ppluint64)buf.st_size);
			throwErrno(filename());
		#endif
	}
	throw FileNotOpenException();
}

/*!\brief Prozess öffnen
 *
 * \desc
 * Die  Funktion \c Popen öffnet einen Prozess dadurch, dass sie sich nach
 * Erzeugen einer Pipe aufteilt und eine Shell öffnet.  Da eine Pipe  nach
 * Definition  unidirektional  ist,  darf das Argument \p mode nur Lesen oder
 * Schreiben angeben,  nicht  beides;  der  resultierende  Datenkanal  ist
 * demzufolge nur-lesend oder nur-schreibend.
 *
 * \param[in] command Das  Argument \p command  enthält einen String,
 * der ein Shell-Kommandozeile enthält.  Dieses Kommando  wird  an \c /bin/sh
 * unter Verwendung des Flags \c -c übergeben. Interpretation, falls nötig, wird von
 * der Shell durchgeführt.
 * \param[in] mode Das Argument \p mode ist ein Zeiger auf einen mit NULL
 * beendeten  String,  der  entweder  ‘r’  für  Lesen  oder  ‘w’ für
 * Schreiben sein muss.
 *
 * @return Bei Erfolg liefert die Funktion 1 zurück, sonst 0
 */
void File::popen (const String &command, FileMode mode)
{
	close();
	if (command.isEmpty()) throw IllegalArgumentException();

	if ((ff=(FILE*)::popen((const char*)command.toLocalEncoding(),fmode(mode)))==NULL) {
		throwErrno(command);
	}
	isPopen=true;
	mysize=size();
	seek(0);
	setFilename(command);
}

/*!\brief Prozess öffnen
 *
 * \desc
 * Die  Funktion \c Popen öffnet einen Prozess dadurch, dass sie sich nach
 * Erzeugen einer Pipe aufteilt und eine Shell öffnet.  Da eine Pipe  nach
 * Definition  unidirektional  ist,  darf das Argument \p mode nur Lesen oder
 * Schreiben angeben,  nicht  beides;  der  resultierende  Datenkanal  ist
 * demzufolge nur-lesend oder nur-schreibend.
 *
 * \param[in] command Das  Argument \p command  enthält einen String,
 * der ein Shell-Kommandozeile enthält.  Dieses Kommando  wird  an \c /bin/sh
 * unter Verwendung des Flags \c -c übergeben. Interpretation, falls nötig, wird von
 * der Shell durchgeführt.
 * \param[in] mode Das Argument \p mode ist ein Zeiger auf einen mit NULL
 * beendeten  String,  der  entweder  ‘r’  für  Lesen  oder  ‘w’ für
 * Schreiben sein muss.
 * \param[in] ... Optionale Parameter
 *
 * @return Bei Erfolg liefert die Funktion 1 zurück, sonst 0
 */
void File::popen (const char * command, FileMode mode, ...)
{
	String f;
	va_list args;
	va_start(args, mode);
	f.vasprintf(command,args);
	va_end(args);
	return popen(f,mode);
}

void File::open (FILE * handle)
/*!\brief Bereits geöffnete Datei übernehmen
 *
 * Mit dieser Funktion kann eine mit der C-Funktion \c fopen bereits geöffnete Datei
 * übernommen werden.
 * @param[in] handle Das Filehandle
 * @return Liefert 1 zurück, wenn das Filehandle erfolgreich übernommen wurde, sonst 0.
 * Ein Fehler kann nur auftreten, wenn das übergebene Filehandle selbst NULL ist.
 */
{
	close();
	if (handle==NULL) throw IllegalArgumentException();
	ff=handle;
	mysize=size();
	seek(0);
	setFilename("FILE");
}

/*!\brief Dateizeiger auf gewünschte Stelle bringen
 *
 * Diese Funktion bewegt den internen Dateizeiger auf die gewünschte Stelle
 *
 * \param[in] position Gewünschte Position innerhalb der Datei
 * \return Liefert 1 zurück, wenn der Dateizeiger erfolgreich auf die gewünschte
 * Position bewegt werden konnte, sonst 0. Bei Auftreten eines Fehlers kann
 * sich die Dateiposition dennoch geändert haben und sollte daher mit File::Ftell
 * abgefragt werden.
 */
ppluint64 File::seek(ppluint64 position)
{
	if (ff!=NULL) {
		#ifdef WIN32FILES
			LARGE_INTEGER in,out;
			in.QuadPart=position;
			if (SetFilePointerEx((HANDLE)ff,in,&out,FILE_BEGIN)) {
				pos=out.QuadPart;
			} else {
				SetError(11);
			}
		#else
			fpos_t p;
			#ifdef FPOS_T_STRUCT
				p.__pos=(__off64_t)position;
			#else
				p=(fpos_t)position;
			#endif
			int ret;
			if ((ret=::fsetpos((FILE*)ff,&p))!=0) {
				int errnosave=errno;
				pos=tell();
				errno=errnosave;
				throwErrno(filename());
			}
			pos=tell();
		#endif
		if (pos!=position) throw FileSeekException();
		return pos;
	}
	throw FileNotOpenException();
}


/*!\brief Dateizeiger auf gewünschte Stelle bringen
 *
 * Die Funktion Fseek setzt den Dateipositionszeiger für den Stream. Die neue Position,
 * gemessen in Byte, wird erreicht durch addieren von  \p offset  zu  der  Position,  die  durch  \p origin
 * angegeben  ist. Wenn \p origin auf SEEK_SET, SEEK_CUR, oder SEEK_END, gesetzt ist, ist der Offset relativ
 * zum Dateianfang, der aktuellen Position, oder dem Dateiende.
 *
 * Ein  erfolgreicher  Aufruf  der  Funktion fseek  löscht  den Dateiendezeiger für den Stream.
 *
 * @param offset Anzahl Bytes, die gesprungen werden soll.
 * @param origin Gibt die Richtung an, in die der Dateizeiger bewegt werden soll. Es kann einen
 * der folgenden Werte annehmen:
 * - SEEK_SET \p offset wird vom Beginn der Datei berechnet
 * - SEEK_CUR \p offset wird von der aktuellen Dateiposition gerechnet
 * - SEEK_END \p offset wird vom Ende der Datei aus nach vorne berechnet
 * @return Bei Erfolg gibt die Funktion die tatsächliche Position zurück,
 * im Fehlerfall wird eine Exception geworfen.
 */
ppluint64 File::seek (pplint64 offset, SeekOrigin origin )
{
	int suberr;
	if (ff!=NULL) {
		#ifdef WIN32FILES
			int o=FILE_BEGIN;
			if (origin==SEEKCUR) o=FILE_CURRENT;
			if (origin==SEEKEND) o=FILE_END;
			if (origin==SEEKSET) o=FILE_BEGIN;
			LARGE_INTEGER in,out;
			in.QuadPart=offset;
			if (SetFilePointerEx((HANDLE)ff,in,&out,o)) {
				pos=out.QuadPart;
				return 1;
			}
			SetError(11);
			return 0;
		#elif defined _HAVE_FSEEKO
			fpos_t p;
			p=(fpos_t) offset;
			suberr=::fseeko((FILE*)ff,p,origin);
		#else
			int o=0;
			switch (origin) {
				case File::SEEKCUR:
					o=SEEK_CUR;
					break;
				case File::SEEKSET:
					o=SEEK_SET;
					break;
				case File::SEEKEND:
					o=SEEK_END;
					break;
				default:
					throw IllegalArgumentException();
			}
			suberr=::fseek((FILE*)ff,(long)offset,o);
		#endif
		if (suberr==0) {
			pos=tell();
			return pos;
		}
		throwErrno(filename());
	}
	throw FileNotOpenException();
	return 0;
}

/*!\brief Aktuelle Dateiposition ermitteln
 *
 * Die Funktion Ftell liefert den aktuellen Wert des Dateipositionszeigers für  den  Stream zurück.
 *
 * @return Position des Zeigers innerhalb der Datei. Im Fehlerfall wird eine
 * Exception geworfen
 */
ppluint64 File::tell()
{
	if (ff!=NULL) {
		#ifdef WIN32FILES
			LARGE_INTEGER in,out;
			in.QuadPart=0;
			if (SetFilePointerEx((HANDLE)ff,in,&out,FILE_CURRENT)) {
				return out.QuadPart;
			}
			SetError(11);
			return 0;
		#else
			fpos_t p;
			if (fgetpos((FILE*)ff,&p)!=0) throwErrno(filename());
			ppluint64 ret;
			#ifdef FPOS_T_STRUCT
				ret=(pplint64)p.__pos;
			#else
				ret=(pplint64)p;
			#endif
			return ret;
		#endif
	}
	throw FileNotOpenException();
}

/*!\brief Lesen eines Datenstroms
 *
 * Die  Funktion  Fread  liest \p nmemb Datenelemente vom Dateistrom und speichert
 * es an  der  Speicherposition,  die  durch \p ptr bestimmt ist.  Jedes davon ist
 * \ size Byte lang.
 *
 * @param[out] ptr Pointer auf den Speicherbereich, in den die gelesenen Daten
 * abgelegt werden sollen. Der Aufrufer muss vorher mindestens \p size * \p nmemb
 * Bytes Speicher reserviert haben.
 * @param[in] size Größe der zu lesenden Datenelemente
 * @param[in] nmemb Anzahl zu lesender Datenelemente
 * @return Fread  gibt die Anzahl der erfolgreich gelesenen Elemente zurück
 * (nicht die Anzahl  der  Zeichen).  Wenn  ein Fehler  auftritt  oder  das
 * Dateiende erreicht ist, wird eine Exception geworfen.
 * \exception EndOfFileException: Wird geworfen, wenn das Dateiende erreicht wurde
 */
size_t File::fread(void * ptr, size_t size, size_t nmemb)
{
	if (ff==NULL) throw FileNotOpenException();
	if (ptr==NULL) throw IllegalArgumentException();
	size_t by=::fread(ptr,size,nmemb,(FILE*)ff);
	pos+=(by*size);
	if (by==0) {
		if (::feof((FILE*)ff)) throw EndOfFileException();
		throwErrno(filename());
	}
	return by;
}

/*!\brief Schreiben eines Datenstroms
 *
 * Die Funktion Fwrite schreibt \p nmemb Datenelemente der Größe \p size Bytes,
 * in  den  Dateistrom. Sie werden von der Speicherstelle, die durch \p ptr angegeben ist, gelesen.
 *
 * @param ptr Pointer auf den Beginn des zu schreibenden Speicherbereiches.
 * @param size Größe der zu schreibenden Datenelemente
 * @param nmemb Anzahl zu schreibender Datenelemente
 * @return Fwrite gibt die Anzahl der erfolgreich geschriebenen Elemente zurück (nicht die
 * Anzahl der Zeichen). Wenn ein Fehler auftritt, wird eine kleinere Zahl oder 0 zurückgegeben.
 * Der Fehler kann auf dem üblichen Weg ausgelesen werden.
 *
 */
size_t File::fwrite(const void * ptr, size_t size, size_t nmemb)
{
	if (ff==NULL) throw FileNotOpenException();
	if (ptr==NULL) throw IllegalArgumentException();
	size_t by=::fwrite(ptr,size,nmemb,(FILE*)ff);
	pos+=(by*size);
	if (pos>this->mysize) this->mysize=pos;
	if (by<nmemb) throwErrno(filename());
	return by;
}

/*!\brief Daten aus einer anderen Datei kopieren
 *
 * Mit dieser Funktion kann ein Datenblock aus einer anderen Datei in diese
 * hineinkopiert werden. Die Daten werden dabei ab dem aktuellen Dateipositionszeiger
 * des \p quellfile an den aktuellen Zeiger dieser Datei kopiert.
 *
 * @param quellfile Das Dateiobjekt, aus dem gelesen werden soll
 * @param bytes Anzahl zu kopierender Bytes
 * @return Bei Erfolg liefert die Funktion die Anzahl kopierter Bytes zurück.
 * Im Fehlerfall kann der Wert auch keiner als die Anzahl Bytes sein oder auch 0.
 *
 * \note Die Funktion verwendet einen internen Buffer zum Zwischenspeichern
 * der gelesenen Daten.
 */
ppluint64 File::doCopy (FileObject &quellfile, ppluint64 bytes)
{
	if (ff==NULL) throw FileNotOpenException();
	if (buffer==NULL) {
		buffer=(char *)malloc(COPYBYTES_BUFFERSIZE);
		if (buffer==NULL) throw OutOfMemoryException();
	}
	if (quellfile.size()>quellfile.tell()) {
		if ((quellfile.tell()+(ppluint64)bytes)>quellfile.size()) {
			bytes=quellfile.size()-quellfile.tell();
		}
		ppluint64 rest=bytes;
		ppluint64 by;
		while (rest>0) {
			by=rest;
			if (by>COPYBYTES_BUFFERSIZE) by=COPYBYTES_BUFFERSIZE;
			by=quellfile.read (buffer,(size_t)by);
			write (buffer,(size_t)by);
			rest-=by;
		}
	}
	return bytes;
}

/*!\brief String lesen
 *
 * %fgets liest höchstens \p num minus ein Zeichen aus der Datei und speichert
 * sie in dem Puffer, auf den \p buffer zeigt. Das Lesen stoppt nach einem
 * EOF oder Zeilenvorschub. Wenn ein Zeilenvorschub gelesen wird, wird
 * er in dem Puffer gespeichert. Am Ende der gelesenen Daten wird ein
 * 0-Byte angehangen.
 *
 * @param buffer Pointer auf den Speicherbereich, in den die gelesenen Daten
 * geschrieben werden sollen. Dieser muss vorher vom Aufrufer allokiert worden
 * sein und mindestens \p num Bytes groß sein.
 * @param num Anzahl zu lesender Zeichen
 * @return Bei Erfolg wird \p buffer zurückgegeben, im Fehlerfall NULL.
 */
char * File::gets (char *buffer, size_t num)
{
	if (ff==NULL) throw FileNotOpenException();
	if (buffer==NULL) throw IllegalArgumentException();
	int suberr;
	char *res;
	res=::fgets(buffer, num, (FILE*)ff);
	if (res==NULL) {
		suberr=::ferror((FILE*)ff);
		if (::feof((FILE*)ff)) throw EndOfFileException();
		else throwErrno(filename());
	}
	ppluint64 by=(ppluint64)strlen(buffer);
	pos+=by;
	return buffer;
}

/*!\brief Wide-Character String lesen
 *
 * Gets liest höchstens \p num minus ein Zeichen (nicht Bytes) aus der Datei
 * und speichert sie in dem Puffer, auf den \p buffer zeigt. Das Lesen stoppt
 * nach einem EOF oder Zeilenvorschub. Wenn ein Zeilenvorschub gelesen wird,
 * wird er in dem Puffer gespeichert. Am Ende der gelesenen Daten wird ein
 * 0-Byte angehangen.
 *
 * @param buffer Pointer auf den Speicherbereich, in den die gelesenen Daten
 * geschrieben werden sollen. Dieser muss vorher vom Aufrufer allokiert worden
 * sein und mindestens \p num * \c sizeof(wchar_t) Bytes groß sein.
 * @param num Anzahl zu lesender Zeichen
 * @return Bei Erfolg wird \p buffer zurückgegeben, im Fehlerfall NULL.
 *
 * \note Die Funktion ist unter Umständen nicht auf jedem Betriebssystem
 * verfügbar. In diesem Fall wird Fehlercode 246 zurückgegeben.
 *
 */
wchar_t *File::getws (wchar_t *buffer, size_t num)
{
	if (ff==NULL) throw FileNotOpenException();
	if (buffer==NULL) throw IllegalArgumentException();
#ifndef HAVE_FGETWS
	throw UnsupportedFeatureException("ppl7::File::getws: No fgetws available");
#else
	int suberr;
	wchar_t *res;
	res=::fgetws(buffer, num, (FILE*)ff);
	if (res==NULL) {
		suberr=::ferror((FILE*)ff);
		if (::feof((FILE*)ff)) throw EndOfFileException();
		else throwErrno(filename());
	}
	ppluint64 by=(ppluint64)wcslen(buffer)*sizeof(wchar_t);
	pos+=by;
	return buffer;
#endif
}

/*!\brief String schreiben
 *
 * Puts schreibt die Zeichenkette \p str ohne sein nachfolgendes 0-Byte in
 * den Ausgabestrom.
 *
 * @param str Pointer auf den zu schreibenden String
 * @return Bei Erfolg wird 1 zurückgegeben, im Fehlerfall 0.
 */
void File::puts (const char *str)
{
	if (ff==NULL) throw FileNotOpenException();
	if (str==NULL) throw IllegalArgumentException();
	if (fputs(str,(FILE*)ff)!=EOF) {
		pos+=strlen(str);
		if (pos>mysize) mysize=pos;
		return;
	}
	throwErrno(filename());
}

/*!\brief Wide-Character String schreiben
 *
 * Puts schreibt die Zeichenkette \p str ohne sein nachfolgendes 0-Byte in
 * den Ausgabestrom.
 *
 * @param str Pointer auf den zu schreibenden String
 * @return Bei Erfolg wird 1 zurückgegeben, im Fehlerfall 0.
 *
 * \note Die Funktion ist unter Umständen nicht auf jedem Betriebssystem
 * verfügbar. In diesem Fall wird Fehlercode 246 zurückgegeben.
 */
void File::putws (const wchar_t *str)
{
	if (ff==NULL) throw FileNotOpenException();
	if (str==NULL) throw IllegalArgumentException();
#ifndef HAVE_FPUTWS
	throw UnsupportedFeatureException("ppl7::File::putws: No fputws available");
#else
	if (fputws(str,(FILE*)ff)!=-1) {
		pos+=wcslen(str)*sizeof(wchar_t);
		if (pos>mysize) mysize=pos;
		return;
	}
	throwErrno(filename());
#endif
}


/*!\brief Zeichen schreiben
 *
 * Putc schreibt das Zeichen \p c, umgesetzt in ein unsigned char,
 * in den Ausgabestrom.
 * @param c Zu schreibendes Zeichen
 * @return Bei Erfolg wird das geschriebende Zeichen als Integer Wert zurückgegeben,
 * im Fehlerfall -1;
 */
void File::putc(int c)
{
	if (ff==NULL) throw FileNotOpenException();
	int	ret=fputc(c,(FILE*)ff);
	if (ret!=EOF) {
		pos++;
		if (pos>mysize) mysize=pos;
		return;
	}
	throwErrno();
}

/*!\brief Zeichen lesen
 *
 * Getc liest das  nächste Zeichen aus der Datei und gibt seinen unsigned char Wert gecastet
 * in einem int zurück.
 * @return Bei Erfolg wird der Wert des gelesenen Zeichens zurückgegeben, im
 * Fehlerfall -1.
 */
int File::getc()
{
	if (ff==NULL) throw FileNotOpenException();
	int ret=fgetc((FILE*)ff);
	if (ret!=EOF) {
		pos++;
		return ret;
	}
	throwErrno();
	return 0;
}

/*!\brief Wide-Character Zeichen schreiben
 *
 * Putwc schreibt das Wide-Character Zeichen \p c in den Ausgabestrom.
 * @param c Zu schreibendes Zeichen
 * @return Bei Erfolg wird das geschriebende Zeichen als Integer Wert zurückgegeben,
 * im Fehlerfall -1;
 *
 * \note Die Funktion ist unter Umständen nicht auf jedem Betriebssystem
 * verfügbar. In diesem Fall wird Fehlercode 246 zurückgegeben.
 */
void File::putwc(wchar_t c)
{
	if (ff==NULL) throw FileNotOpenException();
#ifndef HAVE_FPUTWC
	throw UnsupportedFeatureException("ppl7::File::putwc: No fputwc available");
#else
	wint_t ret=fputwc(c,(FILE*)ff);
	if (ret!=WEOF) {
		pos+=sizeof(wchar_t);
		if (pos>mysize) mysize=pos;
		return;
	}
	throwErrno();
#endif
}

/*!\brief Wide-Character Zeichen lesen
 *
 * Getwc liest das nächste Zeichen aus der Datei und gibt seinen Wert als Integer
 * zurück.
 * @return Bei Erfolg wird das gelesene Zeichen als Integer Wert zurückgegeben,
 * im Fehlerfall -1;
 *
 * \note Die Funktion ist unter Umständen nicht auf jedem Betriebssystem
 * verfügbar. In diesem Fall wird Fehlercode 246 zurückgegeben.
 */
wchar_t File::getwc()
{
	if (ff==NULL) throw FileNotOpenException();
#ifndef HAVE_FGETWC
	throw UnsupportedFeatureException("ppl7::File::putwc: No fputwc available");
#else
	wint_t ret=fgetwc((FILE*)ff);
	if (ret!=WEOF) {
		pos+=sizeof(wchar_t);
		return(wchar_t) ret;
	}
	throwErrno();
	return 0;
#endif
}

/*!\brief Prüfen, ob Dateiende erreicht ist
 *
 * Die Funktion prüft, ob das Dateiende erreicht wurde
 *
 * @return Liefert \c true zurück, wenn das Dateiende erreicht wurde, sonst \c false.
 * Falls die Datei nicht geöffnet war, wird ebenfalls \c false zurückgegeben.
 */
bool File::eof() const
{
	if (ff==NULL) throw FileNotOpenException();
	if (feof((FILE*)ff)!=0) return true;
	return false;
}

/*!\brief Filenummer der Datei
 *
 * Die Funktion liefert den Dateideskriptor als Integer zurück, wie er
 * von den Systemfunktionen open , read , write und close genutzt wird.
 *
 * @return Liefert die Filenummer zurück. Ist keine Datei geöffnet, wird eine
 * Exception geworfen.
 */
int File::getFileNo() const
{
	if (ff==NULL) throw FileNotOpenException();
	return fileno((FILE*)ff);
}


/*!\brief Gepufferte Daten schreiben
 *
 * Die Funktion Flush bewirkt, dass alle gepufferten Daten des aktuellen Streams
 * mittels der zugrundeliegenden write-Funktion geschrieben werden. Der Status
 * des Streams wird dabei nicht berührt. Die Daten werden nicht zwangsweise auch
 * physikalisch auf die Platte geschrieben, sie können noch immer aus Performancegründen
 * vom Kernel oder Treiber gecached werden. Um 100 Prozent sicher zu gehen, kann man
 * die Funktion File::Sync verwenden.
 *
 * @return Bei erfolgreicher Ausführung wird 1 zurückgegeben, ansonsten 0.
 */
void File::flush()
{
	if (ff==NULL) throw FileNotOpenException();
	#ifdef WIN32FILES
		FlushFileBuffers((HANDLE)ff);
	#else
		if (fflush((FILE*)ff)==0) return;
		throwErrno();
	#endif
}

/*!\brief Dateiänderungen sofort auf die Platte schreiben
 *
 * \desc
 * Für gewöhnlich cached das Betriebssysteme Schreibzugriffe auf die Festplatte, um die Performance
 * zu steigern. Je nach Filesystem und Betriebssystem können zwischen Schreibzugriff der Software bis zum
 * tatsächlichen Schreiben auf die Festplatte zwischen einigen wenigen Sekunden bis zu einer Minute vergehen!
 * Tritt in diesem Zeitraum ein System-Crash oder Stromausfall auf, führt dies unweigerlich zu Datenverlust.
 *
 * Ein Aufruf dieser Funktion bewirkt, dass alle Dateiänderungen sofort auf die Festplatte
 * geschrieben werden. Sie sollte daher vor dem Schließen einer kritischen Datei mit File::Close aufgerufen
 * werden, unter Umständen sogar nach jedem Schreibzugriff.
 *
 * @return Die Funktion kehrt erst zurück, wenn alle Daten vollständig geschrieben wurden und liefert dann true (1)
 * zurück. Können die Daten nicht geschrieben werden, liefert sie false (0) zurück und ein entsprechender
 * Fehlercode wird gesetzt.
 *
 * @since Die Funktion wurde mit Version 6.2.5 eingeführt
 */
void File::sync()
{
	if (ff==NULL) throw FileNotOpenException();
#ifdef HAVE_FSYNC
	int ret=fsync(fileno((FILE*)ff));
	if (ret==0) return;
	throwErrno();
#else
	throw UnsupportedFeatureException("ppl7::File::sync: No fsync available");
#endif
}

/*!\brief Datei abschneiden
 *
 * Die Funktionen Truncate bewirkt, dass die aktuell geöffnete Datei auf eine Größe von
 * exakt \p length Bytes abgeschnitten wird.
 *
 * Wenn die Datei vorher größer war, gehen überschüssige Daten verloren. Wenn die Datei
 * vorher kleiner war, wird sie vergrößert und die zusätzlichen Bytes werden als Nullen geschrieben.
 *
 * Der Dateizeiger wird nicht verändert. Die Datei muss zum Schreiben geöffnet sein.
 *
 * @param length Position, an der die Datei abgeschnitten werden soll.
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
void File::truncate(ppluint64 length)
{
	if (ff==NULL) throw FileNotOpenException();
#ifdef HAVE_FTRUNCATE
	int fd=fileno((FILE*)ff);
	int ret=::ftruncate(fd,(off_t)length);
	if (ret==0) {
		mysize=length;
		if (pos>mysize) seek(mysize);
		return;
	}
	throwErrno();
#else
	throw UnsupportedFeatureException("ppl7::File::truncate: No ftruncate available");
#endif
}


/*!\brief Datei exklusiv sperren
 *
 * Mit LockExclusive wird die Datei exklusiv zum Schreiben gesperrt. Andere
 * Prozesse können nicht auf die Datei zugreifen, solange die Sperre besteht.
 *
 * @param block Gibt an, ob die Funktion warten soll (blocken), bis die Datei
 * gesperrt werden kann (block=true) oder sofort mit einer Fehlermeldung
 * zurückkehren soll (block=false).
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 *
* \see Siehe auch File::LockShared und File::Unlock
*/
void File::lockExclusive(bool block)
{
	if (ff==NULL) throw FileNotOpenException();
#if defined HAVE_FCNTL
	int fd=fileno((FILE*)ff);
	int cmd=F_SETLK;
	if (block) cmd=F_SETLKW;
	struct flock f;
	f.l_start=0;
	f.l_len=0;
	f.l_whence=0;
	f.l_pid=getpid();
	f.l_type=F_WRLCK;
	int ret=fcntl(fd,cmd,&f);
	if (ret!=-1) return;
	throwErrno();
#elif defined HAVE_FLOCK
	int fd=fileno((FILE*)ff);
	int flags=LOCK_EX;
	if (!block) flags|=LOCK_NB;
	int ret=flock(fd,flags);
	if (ret==0) return;
	throwErrno();
#else
	throw UnsupportedFeatureException("ppl7::File::unlock: No file locking available");
#endif
}

/*!\brief Datei zum Lesen sperren
 *
 * Mit LockShared wird die Datei zum Lesen gesperrt. Andere Prozesse können weiterhin
 * auf die Datei zugreifen, allerdings ebenfalls nur lesend.
 *
 * @param block Gibt an, ob die Funktion warten soll (blocken), bis die Datei
 * gesperrt werden kann (block=true) oder sofort mit einer Fehlermeldung
 * zurückkehren soll (block=false).
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \see Siehe auch File::LockExclusive und File::Unlock
 */
void File::lockShared(bool block)
{
	if (ff==NULL) throw FileNotOpenException();
#if defined HAVE_FCNTL
	int fd=fileno((FILE*)ff);
	int cmd=F_SETLK;
	if (block) cmd=F_SETLKW;
	struct flock f;
	f.l_start=0;
	f.l_len=0;
	f.l_whence=0;
	f.l_pid=getpid();
	f.l_type=F_RDLCK;
	int ret=fcntl(fd,cmd,&f);
	if (ret!=-1) return;
	throwErrno();

#elif defined HAVE_FLOCK
	int fd=fileno((FILE*)ff);
	int flags=LOCK_SH;
	if (!block) flags|=LOCK_NB;
	int ret=flock(fd,flags);
	if (ret==0) return;
	throwErrno();
#else
	throw UnsupportedFeatureException("ppl7::File::unlock: No file locking available");

#endif
}

void File::unlock()
/*!\brief Dateisperre aufheben
 *
 * Mit Unlock wird eine mit LockShared oder LockExclusive eingerichtete
 * Sperre wieder aufgehoben, so dass auch andere Prozesse wieder uneingeschränkt
 * auf die Datei zugreifen können.
 *
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \see Siehe auch File::LockShared und File::LockExclusive
 */
{
	if (ff==NULL) throw FileNotOpenException();
#if defined HAVE_FCNTL
	int fd=fileno((FILE*)ff);
	struct flock f;
	f.l_start=0;
	f.l_len=0;
	f.l_whence=0;
	f.l_pid=getpid();
	f.l_type=F_UNLCK;
	int ret=fcntl(fd,F_SETLKW,&f);
	if (ret!=-1) return;
	throwErrno();

#elif defined HAVE_FLOCK
	int fd=fileno((FILE*)ff);
	int ret=flock(fd,LOCK_UN);
	if (ret==0) return;
	throwErrno();
#else
	throw UnsupportedFeatureException("ppl7::File::unlock: No file locking available");
#endif
}

void File::setMapReadAhead(size_t bytes)
/*!\brief Minimalgröße des Speicherblocks bei Zugriffen mit FileObject::Map
 *
 * Falls mit Map viele aufeinanderfolgende kleine Speicherblöcke gemapped werden,
 * ist es sinnvoll größere Blöcke zu laden, die dann bereits im Cache bzw. Hauptspeicher
 * liegen, wenn sie gebraucht werden. Mit dieser Funktion kann bestimmt werden, wie
 * viele Daten im Voraus gemapped werden sollen.
 *
 * @param bytes Anzahl Bytes, die im Voraus gemapped werden sollen.
 */
{
	ReadAhead=bytes;
}


/*!\brief Datei in den Speicher mappen
 *
 * \descr
 * Mit dieser Funktion wird ein Teil der Datei in den Speicher gemapped.
 * Falls das Betriebssystem <a href="http://en.wikipedia.org/wiki/Mmap">mmap</a> versteht,
 * wird dieser verwendet. Dabei wird der gewünschte Datenblock nicht sofort komplett
 * in den Speicher geladen, sondern nur der Teil, auf den gerade zugegriffen wird.
 * Steht \c mmap nicht zur Verfügung, wird die Datei in den Hauptspeicher geladen.
 * Die File-Klasse kümmert sich selbst daraum, dass der Speicher auch wieder freigegeben
 * wird.
 * \par
 * Ein mit Map gemappter Speicher darf nur gelesen, aber nicht beschrieben werden! Falls
 * auch geschrieben werden soll, ist die Funktion File::MapRW zu verwenden.
 *
 * @param[in] position Die gewünschte Startposition innerhalb der Datei
 * @param[in] bytes Die Anzahl Bytes, die gemapped werden sollen.
 * @return Bei Erfolg gibt die Funktion einen Pointer auf den Speicherbereich zurück,
 * in dem sich die Datei befindet, im Fehlerfall NULL.
 */
const char *File::map(ppluint64 position, size_t bytes)
{
	if (ff==NULL) throw FileNotOpenException();
	if (position+bytes<=mysize) {
		if (MapBase!=NULL) {
			if (LastMapStart==position) {	// Dateiausschnitt schon gemapped?
				if (bytes<=LastMapSize) return MapBase;
			}
			if (position>LastMapStart && (bytes+position-LastMapStart)<=LastMapSize) return MapBase+position-LastMapStart;
			this->munmap(MapBase, (size_t)LastMapSize);
		}
		LastMapStart=position;
		if (ReadAhead>0 && bytes<ReadAhead) {
			bytes=ReadAhead;
			if (position+(ppluint64)bytes>mysize) bytes=(size_t)(mysize-position);
		}
		LastMapSize=bytes;
		return (const char*)this->mmap(position,bytes,1,0);
	}
	throw OverflowException();
}

/*!\brief Datei Les- und Schreibbar in den Speicher mappen
 *
 * \descr
 * Mit dieser Funktion wird ein Teil der Datei in den Speicher gemapped.
 * Falls das Betriebssystem <a href="http://en.wikipedia.org/wiki/Mmap">mmap</a> versteht,
 * wird dieser verwendet. Dabei wird der gewünschte Datenblock nicht sofort komplett
 * in den Speicher geladen, sondern nur der Teil, auf den gerade zugegriffen wird.
 * Steht \c mmap nicht zur Verfügung, wird die Datei in den Hauptspeicher geladen.
 * Die File-Klasse kümmert sich selbst daraum, dass der Speicher nach Gebrauch wieder
 * zurück in die Datei geschrieben und freigegeben wird.
 * \par
 * Ein mit MapRW gemappter Speicher darf sowohl gelesen als auch beschrieben werden!
 * Bevor mit anderen Funktionen auf den gleichen Speicher zugegriffen werden soll
 * (insbesondere schreibend), muss die Funktion File::Unmap aufgerufen werden.
 *
 *
 * @param[in] position Die gewünschte Startposition innerhalb der Datei
 * @param[in] bytes Die Anzahl Bytes, die gemapped werden sollen.
 * @return Bei Erfolg gibt die Funktion einen Pointer auf den Speicherbereich zurück,
 * in dem sich die Datei befindet, im Fehlerfall NULL.
 */
char *File::mapRW(ppluint64 position, size_t bytes)
{
	if (ff==NULL) throw FileNotOpenException();
	if (position+bytes<=mysize) {
		if (MapBase!=NULL) {
			if ((LastMapProtection&2)) {	// Schon als read/write gemapped?
				if (LastMapStart==position) {	// Dateiausschnitt schon gemapped?
					if (bytes<=LastMapSize) return MapBase;
				}
				if (position>LastMapStart && (bytes+position-LastMapStart)<=LastMapSize) return MapBase+position-LastMapStart;
			}
			this->munmap(MapBase, (size_t)LastMapSize);
		}
		if (ReadAhead>0 && bytes<ReadAhead) {
			bytes=ReadAhead;
			if (position+bytes>mysize) bytes=(size_t)(mysize-position);
		}
		return (char*)this->mmap(position,bytes,3,0);
	}
	throw OverflowException();
}

void File::unmap()
{
	this->munmap(MapBase, (size_t)LastMapSize);
}


int File::munmap(void *addr, size_t len)
{
	if (!addr) return 1;
#ifdef HAVE_MMAP
	::munmap(addr, len);
#else
	if ((LastMapProtection&2)) {			// Speicher war schreibbar und muss
		if (seek(LastMapStart)) {		// Zurückgeschrieben werden
			fwrite(MapBase,1,len);
		}
	}
	free (MapBase);
#endif
	LastMapStart=LastMapSize=0;
	MapBase=NULL;
	LastMapProtection=0;
	return 1;
}


static int __pagesize=0;

void *File::mmap(ppluint64 position, size_t size, int prot, int flags)
{
#ifdef HAVE_MMAP
	int mflags=0;
	if (prot&1) mflags|=PROT_READ;
	if (prot&2) mflags|=PROT_WRITE;
	if (prot&4) mflags|=PROT_EXEC;
	size_t rest=0;
#ifdef HAVE_SYSCONF
	if (!__pagesize) __pagesize=sysconf(_SC_PAGE_SIZE);
	// position muss an einer pagesize aligned sein
	rest=position%__pagesize;
	if (rest) {
		// Wir müssen alignen
		size_t multiplyer=position/__pagesize;
		position=multiplyer*__pagesize;
		size+=rest;
	}
#endif

	void *adr=::mmap(NULL, size, mflags,MAP_PRIVATE, fileno((FILE*)ff), (off_t) position);
	if (adr==MAP_FAILED) {
		MapBase=NULL;
		LastMapSize=0;
		throwErrno();
		return NULL;
	}
	LastMapSize=size;
	LastMapProtection=prot;
	LastMapStart=position;
	MapBase=(char*)adr;
	return (MapBase+rest);

#else
	size_t bytes;
	char *adr=(char*)malloc((size_t)size+1);
	if (!adr) throw OutOfMemoryException();
	if (pos!=position) seek((ppldd)position);
	try {
		bytes=fread(adr,1,size);
	} catch (...) {
		free(adr);
		throw;
	}
	adr[bytes]=0;
	MapBase=adr;
	LastMapSize=bytes;
	LastMapProtection=prot;
	LastMapStart=position;
	return (MapBase);
#endif

}


/*!\brief Geöffnete Datei löschen
 *
 * Mit dieser Funktion wird die aktuelle Datei zunächst geschlossen und dann
 * gelöscht.
 *
 * @return Bei Erfolg liefert die Funktion 1 zurück, sonst 0.
 */
void File::erase()
{
	if (ff==NULL) throw FileNotOpenException();
	String Filename=filename();
	close();
	if (Filename.size()>0) {
		DeleteFile(Filename);
	}
}


#ifdef TODO

// ####################################################################
// Statische Funktionen
// ####################################################################

int File::LoadFile(CVar &object, const char *filename)
/*!\ingroup PPLGroupFileIO
 * \brief Datei öffnen und den kompletten Inhalt in ein Objekt laden
 *
 * Mit dieser Funktion wird eine Datei zum Lesen geöffnet und der komplette Inhalt in das
 * angegebene Objekt geladen. Unterstützt werden zur Zeit folgende Objekte:
 * - CString
 * - CWString
 * - CBinary
 *
 * @param[out] object Das gewünschte Zielobjekt
 * @param[in] filename Der Dateiname
 * @return Liefert 1 zurück, wenn die Datei geöffnet und der Inhalt geladen werden konnte, sonst 0.
 */
{
	File ff;
	if (!ff.Open(filename,"rb")) return 0;
	size_t size=(size_t)ff.Size();
	char *buffer=(char*)malloc((size_t)ff.size+1);
	if (!buffer) {
		SetError(2);
		return 0;
	}
	size_t by=ff.Fread(buffer,1,size);
	if (by!=size) {
		free(buffer);
		return 0;
	}
	buffer[by]=0;
	int t=object.DataType();
	if (t==CVar::CBINARY) {
			CBinary &bin= static_cast< CBinary&>(object);  // Objekt zu CBinary umwandeln
			bin.Set(buffer,by);
			bin.ManageMemory();
			return 1;
	} else if (t==CVar::CSTRING) {
			CString &str= static_cast<CString&>(object);  // Objekt zu CString umwandeln
			str.ImportBuffer((char*)buffer,by+1);
			return 1;
	} else if (t==CVar::CWSTRING) {
			CWString &wstr= static_cast<CWString&>(object);  // Objekt zu CWString umwandeln
			wstr.ImportBuffer((wchar_t*)buffer,by+1);
			return 1;
	}
	SetError(337);
	return 0;
}

void *File::LoadFile(const char *filename, size_t *size)
/*!\ingroup PPLGroupFileIO
 * \brief Kompletten Inhalt einer Datei laden
 *
 * Mit dieser Funktion wird eine Datei zum Lesen geöffnet und der komplette Inhalt in den
 * Speicher geladen.
 *
 * @param[in] filename Der Dateiname
 * @param[out] size Wird ein Pointer auf \p size übergeben, wird darin die Größe der geladenen
 * Datei gespeichert.
 * @return Bei Erfolg liefert die Funktion einen Pointer auf den Speicherbereich zurück, in den
 * die Datei geladen wurde. Der Aufrufer ist dafür verantwortlich, dass der Speicher nach Gebrauch
 * mit \c free wieder freigegeben wird. Im Fehlerfall wird NULL zurückgegeben.
 */
{
	File ff;
	if (!ff.Open(filename,"rb")) return 0;
	size_t s=(size_t)ff.Size();
	char *buffer=(char*)malloc(s+1);
	if (!buffer) {
		SetError(2);
		return NULL;
	}
	size_t by=ff.Fread(buffer,1,s);
	if (by!=s) {
		free(buffer);
		return NULL;
	}
	if (size) *size=by;
	buffer[by]=0;
	return buffer;
}

int File::Truncate(const char *filename, ppluint64 bytes)
/*!\ingroup PPLGroupFileIO
 * \brief Datei abschneiden
 *
 * Die Funktionen Truncate bewirkt, dass die mit \p filename angegebene Datei
 * an der Position \p bytes abgeschnitten wird.
 *
 * Wenn die Datei vorher größer war, gehen überschüssige Daten verloren. Wenn die Datei
 * vorher kleiner war, wird sie vergrößert und die zusätzlichen Bytes werden als Nullen geschrieben.
 *
 * @param filename Der Name der gewünschten Datei
 * @param bytes Position, an der die Datei abgeschnitten werden soll.
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	#ifdef HAVE_TRUNCATE
		// truncate-Funktion vorhanden
		if (truncate(filename,(off_t)bytes)==0) return 1;
		SetError(TranslateErrno(errno),errno);
		return 0;
	#endif
	SetError(246,"File::Truncate");
	return 0;
}

int File::Exists(const char * fmt, ...)
/*!\ingroup PPLGroupFileIO
 * \brief Prüfen, ob eine Datei existiert
 *
 * Mit Exists kann geprüft werden, ob eine Datei im Filesystem vorhanden ist.
 *
 * \param fmt Name der gewünschten Datei oder ein Formatstring, falls der Dateiname anhand
 * von weiteren Parametern zusammengesetzt werden muss
 * \param ... Optionale weitere Parameter, die in den vorhergehenden Formatstring \p fmt eingesetzt
 * werden sollen
 * \return Ist die Datei forhanden, gibt die Funktion 1 zurück, andernfalls 0.
 */
{
	char *buff;
	va_list args;
	va_start(args, fmt);
	if (vasprintf (&buff, (char*)fmt, args)<0 || buff==NULL) {
		// Nicht genuegend RAM
		SetError(2);
		return 0;
	}
	va_end(args);
	int ret=0;
	FILE *fd=NULL;
	//printf ("buffer=%s\n",buff);
	fd=fopen(buff,"rb");		// Versuchen die Datei zu oeffnen
	if (fd) {
		ret=1;
		fclose(fd);
	} else {
		SetError(9,buff);
	}
	free(buff);	// Buffer wieder freigeben
	return ret;
}

int File::CopyFile(const char *oldfile, const char *newfile)
/*!\ingroup PPLGroupFileIO
 * \brief Datei kopieren
 *
 * Mit CopyFile wird die Datei \p oldfile nach \p newfile kopiert. Dazu wird die Quelldatei Stück
 * für Stück zunächst in den Hauptspeicher geladen und dann in die Zieldatei geschrieben. War die Zieldatei
 * \p newfile schon vorhanden, wird sie überschrieben. Anders als bei File::MoveFile oder File::RenameFile
 * müssen die beiden Dateien nicht im gleichen Filesystem liegen.
 *
 * \param oldfile Name der zu kopierenden Datei
 * \param newfile Name der Zieldatei.
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	if (strcmp(oldfile,newfile)==0) return 1;	// Nix zu tun
	File f1, f2;
	if (!f1.Open(oldfile,"rb")) return 0;
	if (!f2.Open(oldfile,"wb")) return 0;
	pplint64 bsize=1024*1024;
	if (f1.Size()<bsize) bsize=f1.Size();
	void *buffer=malloc((size_t)bsize);
	if (!buffer) {
		SetError(2);
		return 0;
	}
	ppluint64 rest=f1.Size();
	ppluint64 bytes, done;
	while (rest) {
		bytes=bsize;
		if (bytes>rest) bytes=rest;
		done=f1.Read(buffer,(int)bytes);
		if (done!=bytes) {
			SetError(TranslateErrno(errno),errno);
			PushError();
			f2.Close();
			unlink(newfile);
			free(buffer);
			PopError();
			return 0;
		}
		done=f2.Write(buffer,(int)bytes);
		if (done!=bytes) {
			SetError(TranslateErrno(errno),errno);
			PushError();
			f2.Close();
			unlink(newfile);
			free(buffer);
			PopError();
			return 0;
		}
		rest-=bytes;
	}
	f1.Close();
	f2.Close();
	free(buffer);
	return 1;
}


int File::MoveFile(const char *oldfile, const char *newfile)
/*!\ingroup PPLGroupFileIO
 * \brief Datei verschieben oder umbenennen
 *
 * Mit dieser Funktion wird die Datei \p oldfile zu \p newfile umbenannt. Sie ist identisch mit
 * File::RenameFile. Beide Funktionen arbeiten am effizientesten, wenn die Zieldatei im gleichen
 * Filesystem liegt, da in diesem Fall nur die Verzeichniseinträge geändert werden müssen.
 * Ist dies nicht der Fall, wird automatisch die wesentlich langsamere Funktion File::CopyFile
 * gefolgt von File::DeleteFile aufgerufen.
 *
 * \param oldfile Name der zu verschiebenden bzw. umzubenennenden Datei
 * \param newfile Neuer Name
 * \return Bei Erfolg wird 1 zurückgegeben, im Fehlerfall 0.
 */
{
	if (strcmp(oldfile,newfile)==0) return 1;	// Nix zu tun
	return RenameFile(oldfile,newfile);
	/*
	if (rename(oldfile,newfile)==0) return 1;
	SetErrorFromErrno("%s => %s",oldfile,newfile);
	return 0;
	*/
}



int File::RenameFile(const char *oldfile, const char *newfile)
/*!\ingroup PPLGroupFileIO
 * \brief Datei verschieben oder umbenennen
 *
 * Mit dieser Funktion wird die Datei \p oldfile zu \p newfile umbenannt. Sie ist identisch mit
 * File::RenameFile. Beide Funktionen arbeiten am effizientesten, wenn die Zieldatei im gleichen
 * Filesystem liegt, da in diesem Fall nur die Verzeichniseinträge geändert werden müssen.
 * Ist dies nicht der Fall, wird automatisch die wesentlich langsamere Funktion File::CopyFile
 * gefolgt von File::DeleteFile aufgerufen.
 *
 * \param oldfile Name der zu verschiebenden bzw. umzubenennenden Datei
 * \param newfile Neuer Name
 * \return Bei Erfolg wird 1 zurückgegeben, im Fehlerfall 0.
 */
{
	if (strcmp(oldfile,newfile)==0) return 1;	// Nix zu tun
	if (rename(oldfile,newfile)==0) {
		FILE *fd=NULL;
		//printf ("buffer=%s\n",buff);
		fd=fopen(oldfile,"rb");		// Versuchen die Datei zu oeffnen
		if (fd) {
			fclose(fd);
			if (unlink(oldfile)==0) return 1;
			SetErrorFromErrno("%s => %s",oldfile,newfile);
			unlink(newfile);
			return 0;
		}
		return 1;
	}
	if (errno==EXDEV) {	// oldfile und newfile befinden sich nicht im gleichen Filesystem.
		if (!File::CopyFile(oldfile,newfile)) return 0;
		if (unlink(oldfile)==0) return 1;
	}
	SetErrorFromErrno("%s => %s",oldfile,newfile);
	return 0;
}

int File::TouchFile(const char *filename, ...)
/*!\ingroup PPLGroupFileIO
 * \brief Leere Datei anlegen oder die Zeitstempel des letzten Zugriffs aktualisieren
 *
 * TouchFile arbeitet ähnlich wie das Unix-Lommando \c touch. Ist die angegebene Datei
 * \p filename noch nicht vorhanden, wird sie als leere Datei angelegt. Ist sie bereits vorhanden,
 * wird der Zeitstempel des letzen Zugriffs aktualisiert.
 *
 * \param filename Name der gewünschten Datei oder ein Formatstring, falls der Dateiname anhand
 * von weiteren Parametern zusammengesetzt werden muss
 * \param ... Optionale weitere Parameter, die in den vorhergehenden Formatstring \p filename eingesetzt
 * werden sollen
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	File ff;
	char *buff=NULL;
	va_list args;
	va_start(args, filename);
	vasprintf (&buff, (char*)filename, args);
	va_end(args);
	if (!buff) {
		return 0;
	}
	int ret=0;
	if (ff.Open(buff,"w")) {
		ret=1;
	}
	free(buff);
	return ret;
}

int File::DeleteFile(const char *filename, ...)
/*!\ingroup PPLGroupFileIO
 * \brief Datei löschen
 *
 * Mit dieser Funktion wird die Datei \p filename vom Datenträger gelöscht.
 *
 * \param filename Name der gewünschten Datei oder ein Formatstring, falls der Dateiname anhand
 * von weiteren Parametern zusammengesetzt werden muss
 * \param ... Optionale weitere Parameter, die in den vorhergehenden Formatstring \p filename eingesetzt
 * werden sollen
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Ein Fehler kann auftreten, wenn die
 * Datei garnicht vorhanden ist oder die notwendigen Zugriffsrechte fehlen.
 */
{
	File ff;
	char *buff=NULL;
	va_list args;
	va_start(args, filename);
	vasprintf (&buff, (char*)filename, args);
	va_end(args);
	if (!buff) {
		return 0;
	}
	int ret=0;
	if (unlink(buff)==0) ret=1;
	else SetErrorFromErrno(buff);
	free(buff);
	return ret;
}

int File::WriteFile(const void *content, size_t size, const char *filename, ...)
/*!\ingroup PPLGroupFileIO
 * \brief Daten in Datei schreiben
 *
 * Mit dieser Funktion werden \p size Bytes aus dem Speicherbereich beginnend bei
 * \p content in die Datei \p filename geschrieben. War die Datei bereits vorhanden,
 * wird sie überschrieben.
 *
 * \param content Pointer auf den Speicherbereich, der in die Datei geschrieben werdem soll
 * \param size Anzahl zu schreibender Bytes
 * \param filename Name der gewünschten Datei oder ein Formatstring, falls der Dateiname anhand
 * von weiteren Parametern zusammengesetzt werden muss
 * \param ... Optionale weitere Parameter, die in den vorhergehenden Formatstring \p filename eingesetzt
 * werden sollen
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	File ff;
	char *buff=NULL;
	va_list args;
	va_start(args, filename);
	vasprintf (&buff, (char*)filename, args);
	va_end(args);
	if (!buff) {
		SetError(2);
		return 0;
	}
	int ret=0;
	if (ff.Open(buff,"wb")) {
		if (ff.Fwrite(content,1,size)==size) ret=1;
	}
	free(buff);
	return ret;
}

int File::WriteFile(const CVar &object, const char *filename, ...)
/*!\ingroup PPLGroupFileIO
 * \brief Daten eines von CVar abgeleiteten Objekts in Datei schreiben
 *
 * Mit dieser Funktion wird der Speicher des von CVar abgeleiteten Objekts \p object
 * in die Datei \p filename geschrieben. War die Datei bereits vorhanden, wird sie überschrieben.
 *
 * \param object Ein von CVar abgeleitetes Objekt. Derzeit werden folgende Objekte unterstützt_
 * - CString
 * - CWString
 * - CBinary
 * \param filename Name der gewünschten Datei oder ein Formatstring, falls der Dateiname anhand
 * von weiteren Parametern zusammengesetzt werden muss
 * \param ... Optionale weitere Parameter, die in den vorhergehenden Formatstring \p filename eingesetzt
 * werden sollen
 * \return Bei Erfolg gibt die Funktion die Anzahl geschriebender Bytes zurück, im Fehlerfall 0.
 */
{
	File ff;
	char *buff=NULL;
	va_list args;
	va_start(args, filename);
	vasprintf (&buff, (char*)filename, args);
	va_end(args);
	if (!buff) {
		SetError(2);
		return 0;
	}
	int ret=0;
	if (ff.Open(buff,"wb")) {
		ret=ff.Write(object);
	}
	free(buff);
	return ret;
}


/*!\namespace FILEATTR
 * \ingroup PPLGroupFileIO
 * \brief Definitionen der Datei-Attribute
 *
 * \desc
 * Der Namespace FILEATTR enthält die Definitionen der Datei-Attribute. Folgende Werte
 * sind enthalten:
 *
 * \copydoc FileAttribute.dox
 *
 * \see ppl7::FileAttr
 * \see ppl7::Chmod
 *
 */

static mode_t translate_fileattr(int attr)
{
	mode_t m=0;
#ifdef _WIN32
	if (attr&FILEATTR::USR_READ) m|=_S_IREAD;
	if (attr&FILEATTR::USR_WRITE) m|=_S_IWRITE;
	if (attr&FILEATTR::GRP_READ) m|=_S_IREAD;
	if (attr&FILEATTR::GRP_WRITE) m|=_S_IWRITE;
	if (attr&FILEATTR::OTH_READ) m|=_S_IREAD;
	if (attr&FILEATTR::OTH_WRITE) m|=_S_IWRITE;

#else
	if (attr&FILEATTR::ISUID) m+=S_ISUID;
	if (attr&FILEATTR::ISGID) m+=S_ISGID;
	if (attr&FILEATTR::STICKY) m+=S_ISVTX;
	if (attr&FILEATTR::USR_READ) m+=S_IRUSR;
	if (attr&FILEATTR::USR_WRITE) m+=S_IWUSR;
	if (attr&FILEATTR::USR_EXECUTE) m+=S_IXUSR;
	if (attr&FILEATTR::GRP_READ) m+=S_IRGRP;
	if (attr&FILEATTR::GRP_WRITE) m+=S_IWGRP;
	if (attr&FILEATTR::GRP_EXECUTE) m+=S_IXGRP;
	if (attr&FILEATTR::OTH_READ) m+=S_IROTH;
	if (attr&FILEATTR::OTH_WRITE) m+=S_IWOTH;
	if (attr&FILEATTR::OTH_EXECUTE) m+=S_IXOTH;
#endif
	return m;
}

int File::FileAttr(int attr, const char *filename, ...)
/*! \brief Setz die Attribute einer exisitierenden Datei
 * \ingroup PPLGroupFileIO
 *
 * \desc
 * Mit dieser Funktion können die Zugriffsattribute einer existierenden Datei
 * gesetzt werden.
 *
 * \param attr Ein Wert, der die Attribute enthält
 * \copydoc FileAttribute.dox
 * \param filename Der Dateinamen oder ein Formatstring für den Dateinamen
 * \param ... Optionale Parameter für den Formatstring
 * \return Bei Erfolg gibt die Funktion true (1) zurück, im Fehlerfall wird ein
 * Fehlercode gesetzt, der mit den PPL-Fehlerfunktionen abgefragt werden kann, und die
 * Funktion gibt false (0) zurück.
 *
 * \see Chmod
 * \version 6.0.16
 */
{
	CString f;
	va_list args;
	va_start(args, filename);
	f.VaSprintf(filename,args);
	va_end(args);
	return Chmod((const char*)f,attr);
}

int File::Chmod(const char *filename, int attr)
/*! \brief Setz die Attribute einer exisitierenden Datei
 * \ingroup PPLGroupFileIO
 *
 * \desc
 * Mit dieser Funktion können die Zugriffsattribute einer existierenden Datei
 * gesetzt werden.
 *
 * \param filename Der Dateinamen
 * \param attr Die Attribute
 * \copydoc FileAttribute.dox
 * \return Bei Erfolg gibt die Funktion true (1) zurück, im Fehlerfall wird ein
 * Fehlercode gesetzt, der mit den PPL-Fehlerfunktionen abgefragt werden kann, und die
 * Funktion gibt false (0) zurück.
 *
 * \see FileAttr
 * \version 6.0.16
 */
{
	mode_t m=translate_fileattr(attr);
#ifdef _WIN32
	if (_chmod(filename,m)==0) return 1;
#else
	if (chmod(filename,m)==0) return 1;
#endif
	SetErrorFromErrno("%s",filename);
	return 0;
}


int File::Stat(const char *filename, CDirEntry &out)
{
	return ppl7::Stat(filename,&out);
}

#endif

} // end of namespace ppl7
