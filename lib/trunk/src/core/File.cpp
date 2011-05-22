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


 /*!\class FileAttr
 * \ingroup PPLGroupFileIO
 * \brief Definitionen der Datei-Attribute
 *
 * \desc
 * Die Klasse FileAttr enthält die Definitionen der Datei-Attribute, die mit
 * File::chmod gesetzt oder mit File::stat ausgelesen werden können.
 *
 */

/*!\enum FileAttr::Attributes
 * \brief Definitionen der Datei-Attribute
 *
 * \desc
 * Diese Enumeration enthält die Definitionen der Datei-Attribute, die mit
 * File::chmod gesetzt oder mit File::stat ausgelesen werden können.
 *
 */

/*!\var FileAttr::Attributes FileAttr::IFFILE
 * \brief reguläre Datei
 */

/*!\var FileAttr::Attributes FileAttr::IFSOCK
 * \brief Socket
 */


/*!\var FileAttr::Attributes FileAttr::IFDIR
 * \brief Verzeichnis
 */

/*!\var FileAttr::Attributes FileAttr::IFLINK
 * \brief Symlink
 */

/*!\var FileAttr::Attributes FileAttr::ISUID
 * \brief SUID-Bit
 */

/*!\var FileAttr::Attributes FileAttr::ISGID
 * \brief SGID-Bit
 *
 * Das SGID-Bit     (ISGID)     hat      verschiedene      besondere
 *      Nutzungsmöglichkeiten:  Für  ein  Verzeichnis bedeutet es, das die BSD-
 *      Semantik Anwendung findet: Dateien, die in ihm  erzeugt  werden,  erben
 *      die  Gruppen-ID  des  Verzeichnisses und nicht die effektive Gruppen-ID
 *      des erzeugenden Prozesses, und dort erzeugte  Verzeichnisse  haben  das
 *      SGID-Bit ebenfalls gesetzt. Für eine Datei, bei der das Bit für Gruppe‐
 *      nausführungsrechte (GRP_EXECUTE) nicht gesetzt ist, bedeutet es erzwungenes
 *      Locken von Datei/Datensatz.
 *
 */

/*!\var FileAttr::Attributes FileAttr::ISVTX
 * \brief Sticky-Bit
 *
 * Auf manchen Dateisystemen darf lediglich der Administrator das Sticky-Bit
 *   setzen. Das Sticky-Bit kann verschiedene Bedeutungen haben,
 *   beispielsweise kann in Verzeichnissen mit gesetztem Sticky-Bit eine
 *   Datei nur vom Dateieigentümer oder dem Administrator (root) gelöscht werden.
 */

/*!\var FileAttr::Attributes FileAttr::STICKY
 * \brief Sticky-Bit
 *
 * Auf manchen Dateisystemen darf lediglich der Administrator das Sticky-Bit
 *   setzen. Das Sticky-Bit kann verschiedene Bedeutungen haben,
 *   beispielsweise kann in Verzeichnissen mit gesetztem Sticky-Bit eine
 *   Datei nur vom Dateieigentümer oder dem Administrator (root) gelöscht werden.
 */

/*!\var FileAttr::Attributes FileAttr::USR_READ
 * \brief Leserechte für Eigentümer
 */

/*!\var FileAttr::Attributes FileAttr::USR_WRITE
 * \brief Schreibrechte für Eigentümer
 */

/*!\var FileAttr::Attributes FileAttr::USR_EXECUTE
 * \brief Ausführrechte  für Eigentümer. Handelt es sich um ein Verzeichnis, darf der Eigentümer das Verzeichnis durchsuchen
 */

/*!\var FileAttr::Attributes FileAttr::GRP_READ
 * \brief Leserechte für Gruppe
 */

/*!\var FileAttr::Attributes FileAttr::GRP_WRITE
 * \brief Schreibrechte für Gruppe
 */

/*!\var FileAttr::Attributes FileAttr::GRP_EXECUTE
 * \brief Ausführrechte  für Gruppe. Handelt es sich um ein Verzeichnis, darf die Gruppe das Verzeichnis durchsuchen
 */

/*!\var FileAttr::Attributes FileAttr::OTH_READ
 * \brief Leserechte für andere
 */

/*!\var FileAttr::Attributes FileAttr::OTH_WRITE
 * \brief Schreibrechte für andere
 */

/*!\var FileAttr::Attributes FileAttr::OTH_EXECUTE
 * \brief Ausführrechte  für andere. Handelt es sich um ein Verzeichnis, dürfen andere das Verzeichnis durchsuchen
 */




/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Konstruktor der Klasse
 */
File::File ()
{
	#ifdef USEWIN32
		hf=0;
	#endif
	MapBase=NULL;
	ff=NULL;
	mysize=pos=0;
	LastMapStart=LastMapSize=0;
	LastMapProtection=0;
	ReadAhead=0;
	isPopen=false;
}

/*!\brief Konstruktor der Klasse mit gleichzeitigem Öffnen einer Datei
 *
 * Konstruktor der Klasse, mit dem gleichzeitig eine Datei geöffnet wird.
 * @param[in] filename Name der zu öffnenden Datei
 * @param[in] mode Zugriffsmodus. Defaultmäßig wird die Datei zum binären Lesen
 * geöffnet (siehe \ref ppl7_File_Filemodi)
 */
File::File (const String &filename, FileMode mode)
{
	MapBase=NULL;
	#ifdef USEWIN32
		hf=0;
	#endif
	ff=NULL;
	mysize=pos=0;
	LastMapStart=LastMapSize=0;
	LastMapProtection=0;
	ReadAhead=0;
	isPopen=false;
	open (filename,mode);
}

/*!\brief Konstruktor mit Übernahme eines C-Filehandles
 *
 * \desc
 * Konstruktor der Klasse mit Übernahme eines C-Filehandles einer bereits mit ::fopen geöffneten Datei.
 *
 * @param[in] handle File-Handle
 */
File::File (FILE * handle)
{
	MapBase=NULL;
	ff=NULL;
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

/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor der Klasse sorgt dafür, dass eine noch geöffnete Datei geschlossen wird und
 * alle Systemresourcen wieder freigegeben werden.
 */

File::~File()
{
	if (ff!=NULL) {
		close();
	}
}

/*!\brief C-Filemode-String
 *
 * \desc
 * Diese interne Funktion gibt den zum Datemodus \p mode passenden C-Filemode-String
 * zurück.
 *
 * @param mode Filemodus aus der Enumeration FileMode
 * @return C-String
 */
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
 * @param e Errorcode aus der errno-Variablen
 * @param filename Dateiname, bei der der Fehler aufgetreten ist
 */
void File::throwErrno(int e,const String &filename)
{
	switch (e) {
		case ENOMEM: throw OutOfMemoryException();
		case EINVAL: throw InvalidArgumentsException();
		case ENOTDIR:
		case ENAMETOOLONG: throw InvalidFileNameException(filename);
		case EACCES:
		case EPERM: throw PermissionDeniedException(filename);
		case ENOENT: throw FileNotFoundException(filename);
#ifdef ELOOP
		case ELOOP: throw TooManySymbolicLinksException(filename);
#endif
		case EISDIR: throw NoRegularFileException(filename);
		case EROFS: throw ReadOnlyException(filename);
		case EMFILE: throw TooManyOpenFilesException();
#ifdef EOPNOTSUPP
		case EOPNOTSUPP: throw UnsupportedFileOperationException(filename);
#endif
		case ENOSPC: throw FilesystemFullException();
#ifdef EDQUOT
		case EDQUOT: throw QuotaExceededException();
#endif
		case EIO: throw IOErrorException();
		case EBADF: throw BadFiledescriptorException();
		case EFAULT: throw BadAddressException();
#ifdef EOVERFLOW
		case EOVERFLOW: throw OverflowException();
#endif
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

/*!\brief Exception anhand errno-Variable werfen
 *
 * \desc
 * Diese Funktion wird intern verwendet, um nach Auftreten eines Fehlers, anhand der globalen
 * "errno"-Variablen die passende Exception zu werfen.
 *
 * @param e Errorcode aus der errno-Variablen
 */
void File::throwErrno(int e)
{
	File::throwErrno(e,filename());
}

/*!\brief Datei öffnen
 *
 * \desc
 * Mit dieser Funktion wird eine Datei zum Lesen, Schreiben oder beides geöffnet.
 * @param[in] filename Dateiname
 * @param mode Zugriffsmodus
 *
 * \return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void File::open (const String &filename, FileMode mode)
{
	close();
	// fopen stuerzt ab, wenn filename leer ist
	if (filename.isEmpty()) throw IllegalArgumentException();
	if ((ff=(FILE*)::fopen((const char*)filename.toLocalEncoding(),fmode(mode)))==NULL) {
		throwErrno(errno,filename);
	}
	mysize=size();
	seek(0);
	setFilename(filename);
}

/*!\brief Datei zum Lesen oder Schreiben öffnen
 *
 * \desc
 * Mit dieser Funktion wird eine Datei zum Lesen, Schreiben oder beides geöffnet.
 *
 * \param filename Dateiname als C-String
 * \param mode String, der angibt, wie die Datei geöffnet werden soll (siehe \ref ppl7_File_Filemodi)
 *
 * \return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void File::open (const char * filename, FileMode mode)
{
	if (filename==NULL || strlen(filename)==0) throw IllegalArgumentException();
	close();
	if ((ff=(FILE*)::fopen(filename,fmode(mode)))==NULL) {
		throwErrno(errno,filename);
	}
	mysize=size();
	seek(0);
	setFilename(filename);
}

/*!\brief Eine temporäre Datei zum Lesen und Schreiben öffnen
 *
 * \desc
 * Diese Funktion erzeugt eine temporäre Datei mit einem eindeutigen Namen.
 * Dieser Name wird aus \p filetemplate erzeugt. Dazu  müssen  die letzten
 * sechs  Buchstaben  des  Parameters template XXXXXX sein, diese werden dann
 * durch eine Zeichenkette ersetzt, die diesen Dateinamen eindeutig  macht.
 * Die  Datei  wird dann mit dem Modus read/write und den Rechten 0666 erzeugt.
 *
 * @param[in] filetemplate Pfad und Vorlage für den zu erstellenden Dateinamen
 * als C-String
 *
 * \return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void File::openTemp(const char *filetemplate)
{
	if (filetemplate==NULL || strlen(filetemplate)==0) throw IllegalArgumentException();
	String t=filetemplate;
	openTemp(t);
}

/*!\brief Eine temporäre Datei zum Lesen und Schreiben öffnen
 *
 * \desc
 * Diese Funktion erzeugt eine temporäre Datei mit einem eindeutigen Namen.
 * Dieser Name wird aus \p filetemplate erzeugt. Dazu  müssen  die letzten
 * sechs  Buchstaben  des  Parameters template XXXXXX sein, diese werden dann
 * durch eine Zeichenkette ersetzt, die diesen Dateinamen eindeutig  macht.
 * Die  Datei  wird dann mit dem Modus read/write und den Rechten 0666 erzeugt.
 *
 * @param[in] filetemplate Pfad und Vorlage für den zu erstellenden Dateinamen
 *
 * \return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void File::openTemp(const String &filetemplate)
{
	#ifdef HAVE_MKSTEMP
		close();
		ByteArray tmpname=filetemplate.toLocalEncoding();

		int f=::mkstemp((char*)((const char*)tmpname));
		if (f<0) throwErrno(errno,filetemplate);
		FILE *ff=::fdopen(f, "r+b");
		if (!ff) throwErrno(errno,filetemplate);
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
		setFilename((const char*)tmpname);
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
 * Wenn  der  Stream  zur  Ausgabe  eingerichtet  war,  werden  gepufferte  Daten  zuerst  durch
 * FileObject::flush
 * geschrieben. Der zugeordnete Datei-Deskriptor wird geschlossen, alle Systemressourcen werden
 * freigegeben.
 *
 * \return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void File::close()
{
	if (MapBase!=NULL) {
		this->munmap(MapBase, (size_t)LastMapSize);
	}

	setFilename(L"");
	if (ff!=NULL) {
		int ret=1;
		if (buffer!=NULL) {
			free (buffer);
			buffer=NULL;
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
		if (ret==0) throwErrno(errno,filename());
		return;
	}
	//throw FileNotOpenException();
}

bool File::isOpen() const
{
	if (ff!=NULL) return true;
	return false;
}


ppluint64 File::size () const
{
	if (ff!=NULL) {
		#ifdef WIN32FILES
			BY_HANDLE_FILE_INFORMATION info;
			if (GetFileInformationByHandle((HANDLE)ff,&info))
				return (((ppluint64)info.nFileSizeHigh)<<32)+info.nFileSizeLow;
			throwErrno(errno,filename());	// ???
		#else
			struct stat buf;
			if ((::fstat (fileno((FILE*)ff),&buf))==0)
				return ((ppluint64)buf.st_size);
			throwErrno(errno,filename());
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
 * \param[in] mode Dateimodus
 * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void File::popen (const String &command, FileMode mode)
{
	close();
	if (command.isEmpty()) throw IllegalArgumentException();

	if ((ff=(FILE*)::popen((const char*)command.toLocalEncoding(),fmode(mode)))==NULL) {
		throwErrno(errno,command);
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
 * \param[in] mode Dateimodus
 *
 * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void File::popen (const char * command, FileMode mode)
{
	if (command==NULL || strlen(command)==0) throw IllegalArgumentException();
	close();
	if ((ff=(FILE*)::popen(command,fmode(mode)))==NULL) {
		throwErrno(errno,command);
	}
	isPopen=true;
	mysize=size();
	seek(0);
	setFilename(command);
}

/*!\brief Bereits geöffnete Datei übernehmen
 *
 * Mit dieser Funktion kann eine mit der C-Funktion \c fopen bereits geöffnete Datei
 * übernommen werden.
 *
 * @param[in] handle Das Filehandle
 * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
 */
void File::open (FILE * handle)
{
	if (handle==NULL) throw IllegalArgumentException();
	close();
	ff=handle;
	mysize=size();
	seek(0);
	setFilename("FILE");
}


void File::rewind ()
{
	if (ff!=NULL) {
		pos=0;
		return;
	}
	throw FileNotOpenException();
}

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
				throwErrno(errno,filename());
			}
			pos=tell();
		#endif
		if (pos!=position) throw FileSeekException();
		return pos;
	}
	throw FileNotOpenException();
}

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
		throwErrno(errno,filename());
	}
	throw FileNotOpenException();
	return 0;
}

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
			if (fgetpos((FILE*)ff,&p)!=0) throwErrno(errno,filename());
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

size_t File::fread(void * ptr, size_t size, size_t nmemb)
{
	if (ff==NULL) throw FileNotOpenException();
	if (ptr==NULL) throw IllegalArgumentException();
	size_t by=::fread(ptr,size,nmemb,(FILE*)ff);
	pos+=(by*size);
	if (by!=nmemb) {
		if (::ferror((FILE*)ff)) throwErrno(errno,filename());
	}
	if (by==0) {
		if (::feof((FILE*)ff)) throw EndOfFileException();
		throwErrno(errno,filename());
	}
	return by;
}

size_t File::fwrite(const void * ptr, size_t size, size_t nmemb)
{
	if (ff==NULL) throw FileNotOpenException();
	if (ptr==NULL) throw IllegalArgumentException();
	size_t by=::fwrite(ptr,size,nmemb,(FILE*)ff);
	pos+=(by*size);
	if (pos>this->mysize) this->mysize=pos;
	if (by<nmemb) throwErrno(errno,filename());
	return by;
}

char * File::fgets (char *buffer, size_t num)
{
	if (ff==NULL) throw FileNotOpenException();
	if (buffer==NULL) throw IllegalArgumentException();
	int suberr;
	char *res;
	res=::fgets(buffer, num, (FILE*)ff);
	if (res==NULL) {
		suberr=::ferror((FILE*)ff);
		if (::feof((FILE*)ff)) throw EndOfFileException();
		else throwErrno(errno,filename());
	}
	ppluint64 by=(ppluint64)strlen(buffer);
	pos+=by;
	return buffer;
}

wchar_t *File::fgetws (wchar_t *buffer, size_t num)
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
		else throwErrno(errno,filename());
	}
	ppluint64 by=(ppluint64)wcslen(buffer)*sizeof(wchar_t);
	pos+=by;
	return buffer;
#endif
}

void File::fputs (const char *str)
{
	if (ff==NULL) throw FileNotOpenException();
	if (str==NULL) throw IllegalArgumentException();
	if (::fputs(str,(FILE*)ff)!=EOF) {
		pos+=strlen(str);
		if (pos>mysize) mysize=pos;
		return;
	}
	throwErrno(errno,filename());
}

void File::fputws (const wchar_t *str)
{
	if (ff==NULL) throw FileNotOpenException();
	if (str==NULL) throw IllegalArgumentException();
#ifndef HAVE_FPUTWS
	throw UnsupportedFeatureException("ppl7::File::putws: No fputws available");
#else
	if (::fputws(str,(FILE*)ff)!=-1) {
		pos+=wcslen(str)*sizeof(wchar_t);
		if (pos>mysize) mysize=pos;
		return;
	}
	throwErrno(errno,filename());
#endif
}


void File::fputc(int c)
{
	if (ff==NULL) throw FileNotOpenException();
	int	ret=::fputc(c,(FILE*)ff);
	if (ret!=EOF) {
		pos++;
		if (pos>mysize) mysize=pos;
		return;
	}
	throwErrno(errno);
}

int File::fgetc()
{
	if (ff==NULL) throw FileNotOpenException();
	int ret=::fgetc((FILE*)ff);
	if (ret!=EOF) {
		pos++;
		return ret;
	}
	throwErrno(errno);
	return 0;
}

void File::fputwc(wchar_t c)
{
	if (ff==NULL) throw FileNotOpenException();
#ifndef HAVE_FPUTWC
	throw UnsupportedFeatureException("ppl7::File::putwc: No fputwc available");
#else
	wint_t ret=::fputwc(c,(FILE*)ff);
	if (ret!=WEOF) {
		pos+=sizeof(wchar_t);
		if (pos>mysize) mysize=pos;
		return;
	}
	throwErrno(errno);
#endif
}

wchar_t File::fgetwc()
{
	if (ff==NULL) throw FileNotOpenException();
#ifndef HAVE_FGETWC
	throw UnsupportedFeatureException("ppl7::File::putwc: No fputwc available");
#else
	wint_t ret=::fgetwc((FILE*)ff);
	if (ret!=WEOF) {
		pos+=sizeof(wchar_t);
		return(wchar_t) ret;
	}
	throwErrno(errno);
	return 0;
#endif
}

bool File::eof() const
{
	if (ff==NULL) throw FileNotOpenException();
	if (feof((FILE*)ff)!=0) return true;
	return false;
}

int File::getFileNo() const
{
	if (ff==NULL) throw FileNotOpenException();
	return fileno((FILE*)ff);
}


void File::flush()
{
	if (ff==NULL) throw FileNotOpenException();
	#ifdef WIN32FILES
		FlushFileBuffers((HANDLE)ff);
	#else
		if (fflush((FILE*)ff)==0) return;
		throwErrno(errno);
	#endif
}

void File::sync()
{
	if (ff==NULL) throw FileNotOpenException();
#ifdef HAVE_FSYNC
	int ret=fsync(fileno((FILE*)ff));
	if (ret==0) return;
	throwErrno(errno);
#else
	throw UnsupportedFeatureException("ppl7::File::sync: No fsync available");
#endif
}

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
	throwErrno(errno);
#else
	throw UnsupportedFeatureException("ppl7::File::truncate: No ftruncate available");
#endif
}


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
	throwErrno(errno);
#elif defined HAVE_FLOCK
	int fd=fileno((FILE*)ff);
	int flags=LOCK_EX;
	if (!block) flags|=LOCK_NB;
	int ret=flock(fd,flags);
	if (ret==0) return;
	throwErrno(errno);
#else
	throw UnsupportedFeatureException("ppl7::File::unlock: No file locking available");
#endif
}

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
	throwErrno(errno);

#elif defined HAVE_FLOCK
	int fd=fileno((FILE*)ff);
	int flags=LOCK_SH;
	if (!block) flags|=LOCK_NB;
	int ret=flock(fd,flags);
	if (ret==0) return;
	throwErrno(errno);
#else
	throw UnsupportedFeatureException("ppl7::File::unlock: No file locking available");

#endif
}

void File::unlock()
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
	throwErrno(errno);

#elif defined HAVE_FLOCK
	int fd=fileno((FILE*)ff);
	int ret=flock(fd,LOCK_UN);
	if (ret==0) return;
	throwErrno(errno);
#else
	throw UnsupportedFeatureException("ppl7::File::unlock: No file locking available");
#endif
}

void File::setMapReadAhead(size_t bytes)
{
	ReadAhead=bytes;
}


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
		throwErrno(errno);
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
	if (pos!=position) seek(position);
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
 * \desc
 * Mit dieser Funktion wird die aktuelle Datei zunächst geschlossen und dann
 * gelöscht.
 *
 * @return Kein Rückgabewert, im Fehlerfall wird eine Exception geworfen.
 */
void File::erase()
{
	if (ff==NULL) throw FileNotOpenException();
	String Filename=filename();
	close();
	if (Filename.size()>0) {
		remove(Filename);
	}
}


// ####################################################################
// Statische Funktionen
// ####################################################################

/*!\ingroup PPLGroupFileIO
 * \brief Datei öffnen und den kompletten Inhalt in ein Objekt laden
 *
 * \desc
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
void File::load(ByteArray &object, const String &filename)
{
	load(object,(const char*)filename.toLocalEncoding());
}

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
void File::load(ByteArray &object, const char *filename)
{
	if (filename==NULL || strlen(filename)==0) throw IllegalArgumentException();
	File ff;
	ff.open(filename);
	char *buffer=(char*)malloc((size_t)ff.mysize+1);
	if (!buffer) throw OutOfMemoryException();
	size_t by=ff.fread(buffer,1,ff.mysize);
	if (by!=ff.mysize) {
		free(buffer);
		throw ReadException();
	}
	buffer[by]=0;
	object.use(buffer,by);
}

void *File::load(const String &filename, size_t *size)
{
	return load((const char*)filename.toLocalEncoding(),size);
}

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
void *File::load(const char *filename, size_t *size)
{
	if (filename==NULL || strlen(filename)==0) throw IllegalArgumentException();
	File ff;
	ff.open(filename);
	char *buffer=(char*)malloc((size_t)ff.mysize+1);
	if (!buffer) throw OutOfMemoryException();
	size_t by=ff.fread(buffer,1,ff.mysize);
	if (by!=ff.mysize) {
		free(buffer);
		throw ReadException();
	}
	if (size) *size=by;
	buffer[by]=0;
	return buffer;
}

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
void File::truncate(const String &filename, ppluint64 bytes)
{
#ifdef HAVE_TRUNCATE
	// truncate-Funktion vorhanden
	if (::truncate((const char*)filename.toLocalEncoding(),(off_t)bytes)==0) return;
	throwErrno(errno,filename);
#else
	throw UnsupportedFeatureException("ppl7::File::unlock: No file locking available");
#endif
}

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
void File::truncate(const char*filename, ppluint64 bytes)
{
	if (filename==NULL || strlen(filename)==0) throw IllegalArgumentException();
#ifdef HAVE_TRUNCATE
	// truncate-Funktion vorhanden
	if (::truncate(filename,(off_t)bytes)==0) return;
	throwErrno(errno,filename);
#else
	throw UnsupportedFeatureException("ppl7::File::unlock: No file locking available");
#endif
}

/*!\ingroup PPLGroupFileIO
 * \brief Prüfen, ob eine Datei existiert
 *
 * \desc
 * Mit %exists kann geprüft werden, ob eine Datei im Filesystem vorhanden ist.
 *
 * \param filename Name der gewünschten Datei
 * werden sollen
 * \return Ist die Datei forhanden, gibt die Funktion \c true zurück, andernfalls \c false.
 */
bool File::exists(const String &filename)
{
	FILE *fd=NULL;
	//printf ("buffer=%s\n",buff);
	fd=fopen((const char*)filename.toLocalEncoding(),"rb");		// Versuchen die Datei zu oeffnen
	if (fd) {
		fclose(fd);
		return true;
	}
	return false;
}

/*!\ingroup PPLGroupFileIO
 * \brief Prüfen, ob eine Datei existiert
 *
 * \desc
 * Mit %exists kann geprüft werden, ob eine Datei im Filesystem vorhanden ist.
 *
 * \param filename Name der gewünschten Datei
 * werden sollen
 * \return Ist die Datei forhanden, gibt die Funktion \c true zurück, andernfalls \c false.
 */
bool File::exists(const char *filename)
{
	if (filename==NULL || strlen(filename)==0) throw IllegalArgumentException();
	FILE *fd=NULL;
	//printf ("buffer=%s\n",buff);
	fd=fopen(filename,"rb");		// Versuchen die Datei zu oeffnen
	if (fd) {
		fclose(fd);
		return true;
	}
	return false;
}

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
 * \exception
 */
void File::copy(const String &oldfile, const String &newfile)
{
	if (oldfile==newfile) throw IllegalArgumentException();
	File f1, f2;
	f1.open(oldfile,READ);
	f2.open(oldfile,WRITE);
	ppluint64 bsize=1024*1024;
	if (f1.mysize<bsize) bsize=f1.mysize;
	void *buffer=malloc((size_t)bsize);
	if (!buffer) throw OutOfMemoryException();
	ppluint64 rest=f1.mysize;
	ppluint64 bytes, done;
	while (rest) {
		bytes=bsize;
		if (bytes>rest) bytes=rest;
		done=f1.fread(buffer,1,bytes);
		if (done!=bytes) {
			// Sollte eigentlich nicht vorkommen
			f2.close();
			remove(newfile);
			free(buffer);
			throw ReadException();
		}
		done=f2.fwrite(buffer,1,bytes);
		rest-=bytes;
	}
	f1.close();
	f2.close();
	free(buffer);
}

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
void File::move(const String &oldfile, const String &newfile)
{
	if (oldfile==newfile) throw IllegalArgumentException();
	rename(oldfile,newfile);
}

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
void File::rename(const char *oldfile, const char *newfile)
{
	if (oldfile==NULL || newfile==NULL) throw NullPointerException();
	if (strcmp(oldfile,newfile)==0) throw IllegalArgumentException();
	String desc;
	desc.setf("rename %s => %s",oldfile,newfile);
	if (::rename(oldfile,newfile)==0) {
		FILE *fd=NULL;
		//printf ("buffer=%s\n",buff);
		fd=fopen(oldfile,"rb");		// Ist die alte Datei noch da?
		if (fd) {
			// Ja, wir löschen sie manuell
			fclose(fd);
			if (unlink(oldfile)==0) return;
			int saveerrno=errno;
			unlink(newfile);
			errno=saveerrno;
			throwErrno(errno,desc);
		}
		return;
	}
	if (errno==EXDEV) {	// oldfile und newfile befinden sich nicht im gleichen Filesystem.
		copy(oldfile,newfile);
		if (unlink(oldfile)==0) return;
	}
	throwErrno(errno,desc);
}

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
void File::rename(const String &oldfile, const String &newfile)
{
	File::rename((const char*)oldfile.toLocalEncoding(),(const char*)newfile.toLocalEncoding());
}

/*!\ingroup PPLGroupFileIO
 * \brief Datei löschen
 *
 * Mit dieser Funktion wird die Datei \p filename vom Datenträger gelöscht.
 *
 * \param filename Name der gewünschten Datei
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Ein Fehler kann auftreten, wenn die
 * Datei garnicht vorhanden ist oder die notwendigen Zugriffsrechte fehlen.
 */
void File::remove(const char *filename)
{
	if (!filename) throw NullPointerException();
	if (::unlink(filename)==0) return;
	throwErrno(errno,filename);
}

/*!\ingroup PPLGroupFileIO
 * \brief Datei löschen
 *
 * Mit dieser Funktion wird die Datei \p filename vom Datenträger gelöscht.
 *
 * \param filename Name der gewünschten Datei
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Ein Fehler kann auftreten, wenn die
 * Datei garnicht vorhanden ist oder die notwendigen Zugriffsrechte fehlen.
 */
void File::remove(const String &filename)
{
	remove((const char*)filename.toLocalEncoding());
}

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
void File::touch(const char *filename)
{
	if (!filename) throw NullPointerException();
	File ff;
	ff.open(filename,READWRITE);
}

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
void File::touch(const String &filename)
{
	touch((const char*)filename.toLocalEncoding());
}


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
void File::save(const void *content, size_t size, const char *filename)
{
	File ff;
	ff.open(filename,WRITE);
	ff.fwrite(content,1,size);
}

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
void File::save(const ByteArrayPtr &object, const char *filename)
{
	File ff;
	ff.open(filename,WRITE);
	ff.fwrite(object.ptr(),1,object.size());
}


static mode_t translate_FileAttr(FileAttr::Attributes attr)
{
	mode_t m=0;
#ifdef _WIN32
	if (attr&FileAttr::USR_READ) m|=_S_IREAD;
	if (attr&FileAttr::USR_WRITE) m|=_S_IWRITE;
	if (attr&FileAttr::GRP_READ) m|=_S_IREAD;
	if (attr&FileAttr::GRP_WRITE) m|=_S_IWRITE;
	if (attr&FileAttr::OTH_READ) m|=_S_IREAD;
	if (attr&FileAttr::OTH_WRITE) m|=_S_IWRITE;

#else
	if (attr&FileAttr::ISUID) m+=S_ISUID;
	if (attr&FileAttr::ISGID) m+=S_ISGID;
	if (attr&FileAttr::STICKY) m+=S_ISVTX;
	if (attr&FileAttr::USR_READ) m+=S_IRUSR;
	if (attr&FileAttr::USR_WRITE) m+=S_IWUSR;
	if (attr&FileAttr::USR_EXECUTE) m+=S_IXUSR;
	if (attr&FileAttr::GRP_READ) m+=S_IRGRP;
	if (attr&FileAttr::GRP_WRITE) m+=S_IWGRP;
	if (attr&FileAttr::GRP_EXECUTE) m+=S_IXGRP;
	if (attr&FileAttr::OTH_READ) m+=S_IROTH;
	if (attr&FileAttr::OTH_WRITE) m+=S_IWOTH;
	if (attr&FileAttr::OTH_EXECUTE) m+=S_IXOTH;
#endif
	return m;
}

/*! \brief Setz die Attribute einer exisitierenden Datei
 * \ingroup PPLGroupFileIO
 *
 * \desc
 * Mit dieser Funktion können die Zugriffsattribute einer existierenden Datei
 * gesetzt werden.
 *
 * \param filename Der Dateinamen
 * \param attr Die Attribute
 * \return Bei Erfolg gibt die Funktion true (1) zurück, im Fehlerfall wird ein
 * Fehlercode gesetzt, der mit den PPL-Fehlerfunktionen abgefragt werden kann, und die
 * Funktion gibt false (0) zurück.
 *
 * \see FileAttr
 * \version 6.0.16
 */
void File::chmod(const char *filename, FileAttr::Attributes attr)
{
	mode_t m=translate_FileAttr(attr);
#ifdef _WIN32
	if (_chmod(filename,m)==0) return;
#else
	if (::chmod(filename,m)==0) return;
#endif
	throwErrno(errno,filename);
}

/*!\brief Informationen zu einer Datei oder Verzeichnis
 *
 * \desc
 * Mit dieser statischen Funktion können Informationen zu einer Datei oder einem
 * Verzeichnis ausgelesen werden.
 *
 * @param filename Dateiname
 * @param out Objekt, in dem die Daten gespeichert werden
 * \throw FileNotFoundException: Datei oder Verzeichnis nicht vorhanden
 */
void File::stat(const String &filename, DirEntry &result)
{
	stat((const char*)filename.toLocalEncoding(),result);
}

/*!\brief Informationen zu einer Datei oder Verzeichnis
 *
 * \desc
 * Mit dieser statischen Funktion können Informationen zu einer Datei oder einem
 * Verzeichnis ausgelesen werden.
 *
 * @param filename Dateiname
 * @param out Objekt, in dem die Daten gespeichert werden
 * \throw NullPointerException: Wird geworfen, wenn \p filename auf NULL zeigt
 * \throw FileNotFoundException: Datei oder Verzeichnis nicht vorhanden
 */
void File::stat(const char *filename, DirEntry &out)
{
	if (!filename) throw NullPointerException();
#ifdef HAVE_STAT
	struct stat st;
	if (::stat(filename,&st)!=0) throwErrno(errno,filename);
#elif defined _WIN32
	struct _stat st;
	String File=filename;
	File.replace("/","\\");
	if (_stat((const char*)File.toLocalEncoding(),&st)!=0) throwErrno(errno,filename);
#endif
	out.ATime.setTime_t(st.st_atime);
	out.CTime.setTime_t(st.st_ctime);
	out.MTime.setTime_t(st.st_mtime);
	out.Attrib=FileAttr::NONE;
	out.Size=st.st_size;
	out.File.set(filename);
	out.Path=File::getPath(out.File);
	out.Filename=File::getFilename(out.File);
	out.AttrStr.set(L"----------");
	out.Uid=st.st_uid;
	out.Gid=st.st_gid;
#ifndef WIN32
	out.Blocks=st.st_blocks;
	out.BlockSize=st.st_blksize;
#else
	out.Blocks=0;
	out.BlockSize=0;
#endif
	out.NumLinks=st.st_nlink;

	if ((st.st_mode & S_IFDIR)==S_IFDIR) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::IFDIR);
	if ((st.st_mode & S_IFREG)==S_IFREG) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::IFFILE);
#ifdef S_IFLNK
	if ((st.st_mode & S_IFLNK)==S_IFLNK) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::IFLINK);
#endif
#ifdef S_IFSOCK
	if ((st.st_mode & S_IFSOCK)==S_IFSOCK) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::IFSOCK);
#endif

#ifdef _WIN32
	if (st.st_mode & _S_IREAD) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::USR_READ);
	if (st.st_mode & _S_IWRITE) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::USR_WRITE);
	if (st.st_mode & _S_IEXEC) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::USR_EXECUTE);
#else
	if (st.st_mode & S_IRUSR) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::USR_READ);
	if (st.st_mode & S_IWUSR) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::USR_WRITE);
	if (st.st_mode & S_IXUSR) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::USR_EXECUTE);
	if (st.st_mode & S_ISUID) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::ISUID);

	if (st.st_mode & S_IRGRP) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::GRP_READ);
	if (st.st_mode & S_IWGRP) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::GRP_WRITE);
	if (st.st_mode & S_IXGRP) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::GRP_EXECUTE);
	if (st.st_mode & S_ISGID) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::ISGID);

	if (st.st_mode & S_IROTH) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::OTH_READ);
	if (st.st_mode & S_IWOTH) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::OTH_WRITE);
	if (st.st_mode & S_IXOTH) out.Attrib=(FileAttr::Attributes)(out.Attrib|FileAttr::OTH_EXECUTE);
#endif

	if (out.Attrib&FileAttr::IFLINK) out.AttrStr.set(0,L'l');
	if (out.Attrib&FileAttr::IFDIR) out.AttrStr.set(0,L'd');


	if (out.Attrib&FileAttr::USR_READ) out.AttrStr.set(1,L'r');
	if (out.Attrib&FileAttr::USR_WRITE) out.AttrStr.set(2,L'w');
	if (out.Attrib&FileAttr::USR_EXECUTE) out.AttrStr.set(3,L'x');
	if (out.Attrib&FileAttr::ISUID) out.AttrStr.set(3,L's');

	if (out.Attrib&FileAttr::GRP_READ) out.AttrStr.set(4,L'r');
	if (out.Attrib&FileAttr::GRP_WRITE) out.AttrStr.set(5,L'w');
	if (out.Attrib&FileAttr::GRP_EXECUTE) out.AttrStr.set(6,L'x');
	if (out.Attrib&FileAttr::ISGID) out.AttrStr.set(6,L's');

	if (out.Attrib&FileAttr::OTH_READ) out.AttrStr.set(7,L'r');
	if (out.Attrib&FileAttr::OTH_WRITE) out.AttrStr.set(8,L'w');
	if (out.Attrib&FileAttr::OTH_EXECUTE) out.AttrStr.set(9,L'x');
}

/*!\brief Pfad ohne Dateinamen
 *
 * \desc
 * Diese Funktion liefert den Verzeichnisnamen eines Strings zurück, der Pfad und Dateinamen
 * enthält. Lautet der String beispielsweise "/home/patrick/svn/ppl7/README.TXT" liefert die Funktion
 * "/home/patrick/svn/ppl7/" zurück.
 *
 * @param path Pfad mit Dateinamen
 * @return String mit dem Pfad
 */
String File::getPath(const String &path)
{
	size_t i,l,pos;
	char c;
	l=path.len();
	pos=0;
	for (i=0;i<l;i++) {
		c=path[i];
		if (c=='/' || c==':' || c=='\\') pos=i;
	}
	return path.left(pos);
}

/*!\brief Dateinamen ohne Pfad
 *
 * \desc
 * Diese Funktion liefert den Dateinamen eines Strings zurück, der Pfad und Dateinamen
 * enthält. Lautet der String beispielsweise "/home/patrick/svn/ppl7/README.TXT" liefert die Funktion
 * "README.TXT" zurück.
 *
 * @param path Pfad mit Dateinamen
 * @return String mit dem Dateinamen
 */
String File::getFilename(const String &path)
{
	size_t i,l,pos;
	char c;
	l=path.len();
	pos=0;
	for (i=0;i<l;i++) {
		c=path[i];
		if (c=='/' || c==':' || c=='\\') pos=i+1;
	}
	return path.mid(pos);
}


} // end of namespace ppl7