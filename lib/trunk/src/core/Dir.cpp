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

#ifdef HAVE_DIRENT_H
#include <dirent.h>
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

#include <map>

#ifdef _WIN32
#include <io.h>
#define WIN32_LEAN_AND_MEAN		// Keine MFCs
#include <windows.h>

#endif
#include "ppl7.h"

namespace ppl7 {


/*!\class Dir
 * \ingroup PPLGroupFileIO
 * \brief Klasse zum Durchsuchen von Verzeichnissen
 *
 * \desc
 * Die Klasse Dir wird zum Lesen von Verzeichnissen verwendet. Dazu muss man zunächst mit
 * Dir::open ein Verzeichnis öffnen oder das gewünschte Verzeichnis gleich im Konstruktor
 * angeben. Anschließend kann man mit den Funktionen Dir::getFirst und Dir::getNext die
 * einzelnen Dateien auslesen. Diese sind bereits nach Dateiname sortiert. Mit dem
 * Befehl Dir::resort kann man aber jederzeit eine andere Sortierung einstellen oder diese
 * ganz abschalten.
 * \par
 * Möchte man nur Dateien, die einem bestimmten Muster (Pattern) entsprechen, kann man
 * statt Dir::getFirst und Dir::getNext auch Dir::getFirstPattern und Dir::getNextPattern
 * verwenden, oder falls mal Regular Expressions verwenden möchte, Dir::getFirstRegExp und
 * Dir::getNextRegExp.
 *
 */

/*!\enum Dir::Sort
 * \brief Sortiermöglichkeiten
 *
 * In dieser Enumeration sind die verschiedenen Sortiermöglichkeiten definiert:
 */

/*!\var Dir::Sort Dir::SORT_NONE
 * Keine Sortierung. Die Reihenfolge der Dateien hängt fom Betriebs- und Filesystem ab.
 */

/*!\var Dir::Sort Dir::SORT_FILENAME
 * Es wird eine Sortierung anhand der Dateinamen vorgenommen. Dabei wird Groß- und Kleinschreibung
 * beachtet. Dateien, die mit einem Großbuchstaben beginnen, werden zuerst aufgelistet, danach
 * Dateien mit Kleinbuchstaben.
 */

/*!\var Dir::Sort Dir::SORT_FILENAME_IGNORCASE
 * Es wird eine Sortierung anhand der Dateinamen vorgenommen. Dabei wird Groß- und Kleinschreibung
 * ignoriert. Dateien mit Großbuchstaben und Kleinbuchstaben werden vermischt ausgegeben, wobei
 * jedoch die Alphabetische Reihenfolge erhalten bleibt.
 */

/*!\var Dir::Sort Dir::SORT_ATIME
 * Es wird eine Sortierung nach dem Datum des letzten Zugriffs vorgenommen.
 */

/*!\var Dir::Sort Dir::SORT_MTIME
 * Es wird eine Sortierung nach dem Datum der letzten Modifikation vorgenommen.
 */

/*!\var Dir::Sort Dir::SORT_CTIME
 * Es wird eine Sortierung nach dem Datum der letzten Modifikation vorgenommen.
 */

/*!\var Dir::Sort Dir::SORT_SIZE
 * Es wird eine Sortierung nach der Größe der Datei vorgenommen.
 */


/*!\ingroup PPLGroupFileIO
 * \brief Aktuelles Verzeichnis
 *
 * Diese Funktion liefert das aktuelle Verzeichnis zurück.
 *
 * \return String mit dem aktuellen Verzeichnis
 */
String Dir::currentPath()
{
	String ret;
	char *buf=(char*)malloc(2048);
	if (!buf) throw OutOfMemoryException();
#ifdef _WIN32
	if (_getcwd(buf,2048)) {
#else
	if (getcwd(buf,2048)) {
#endif
		ret=buf;
		free(buf);
		return ret;
	}
	int e=errno;
	free(buf);
	switch (e) {
		case EINVAL: throw IllegalArgumentException();
		case ENOENT: throw NonexistingPathException();
		case ENOMEM: throw OutOfMemoryException();
		case ERANGE: throw PathnameTooLongException();
		case EACCES: throw PermissionDeniedException();
		default: throw UnknownException();
	}
}

/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Konstruktor der Klasse, ohne ein Verzeichnis zu öffnen.
 */
Dir::Dir()
{
	sort=SORT_NONE;
}

/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Der Konstruktor sorgt dafür, dass die internen Variablen und Datenstrukturen
 * initialisert werden. Optional kann auch schon mit dem Parameter \p path ein
 * zu öffnender Pfad angegeben werden und mit \p s eine Sortierreihenfolge.
 *
 * @param[in] path Zu öffnender Pfad (siehe auch CDir::Open)
 * @param[in] s gewünschte Sortierreihenfolge. Defaultmäßig wird keine Sortierung
 * verwendet.
 */
Dir::Dir(const char *path, Sort s)
{
	sort=s;
	if (path) open(path,s);
}



/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor sorgt dafür, dass der intern reservierte Speicher wieder
 * freigegeben wird.
 */
Dir::~Dir()
{
	clear();
}

/*!\brief Verzeichnisliste löschen
 *
 * \desc
 * Wird diese Funktion nach Dir::open aufgerufen, wird die interne Dateiliste
 * wieder gelöscht und der durch die Klasse belegte Speicher freigegeben.
 * Die Funktion wird automatisch vom Destruktor und zu Beginn
 * von Dir::open, so dass sich ein manueller Aufruf der Funktion in der Regel
 * erübrigt.
 */
void Dir::clear()
{
	Files.clear();
	SortedFiles.clear();
	Path.clear();
	sort=SORT_NONE;
}

void Dir::open(const char *path, Sort s)
{
#if defined _WIN32
	clear();
	sort=s;
	Path=path;
	Path.Trim();
	Path.RTrim("/");
	Path.RTrim("\\");
	//printf ("Path=%s \n",(char*)Path);
	CWString Pattern=Path;
	Pattern+="/*";
	struct _wfinddatai64_t data;
	memset(&data,0,sizeof(data));

	intptr_t handle=_wfindfirsti64 ((const wchar_t*)Pattern, &data);
	if (handle<0) {
		SetErrorFromErrno();
		return 0;
	}
	while (1==1) {
		CDirEntry *de=new CDirEntry;
		if (!de) {
			_findclose(handle);
			SetError(2);
			return 0;
		}
		strcpy (de->AttrStr,"----------");
		de->Attrib=0;
		de->ATime=de->CTime=de->MTime=0;

		de->Filename=data.name;
		de->Size=0;
		de->Path=Path;
		de->File=de->Path+CString("/")+de->Filename;
		de->Size=data.size;
		de->Uid=0;
		de->Gid=0;
		de->Blocks=0;
		de->BlockSize=0;
		de->NumLinks=1;

		if (data.attrib & _A_RDONLY) de->Attrib|=CPPLDIR_READONLY;
		if (data.attrib & _A_HIDDEN) de->Attrib|=CPPLDIR_HIDDEN;
		if (data.attrib & _A_SYSTEM) de->Attrib|=CPPLDIR_SYSTEM;
		if (data.attrib & _A_ARCH) de->Attrib|=CPPLDIR_ARCHIV;
		if (data.attrib & _A_SUBDIR) de->Attrib|=CPPLDIR_DIR;

		de->AttrStr[1]=de->AttrStr[4]=de->AttrStr[7]='r';
		if (de->Filename.Len()>4) {
			CString suf=de->Filename.Right(4);
			suf.LCase();
			if (suf.StrCmp(".exe")==0 ||
				suf.StrCmp(".com")==0 ||
				suf.StrCmp(".bat")==0 ) de->AttrStr[3]=de->AttrStr[6]=de->AttrStr[9]='x';
		}

		if (! (de->Attrib&CPPLDIR_READONLY)) de->AttrStr[2]=de->AttrStr[5]=de->AttrStr[8]='w';
		if ( ( de->Attrib & (CPPLDIR_DIR|CPPLDIR_LINK) )==0) de->Attrib|=CPPLDIR_FILE;
		de->ATime=data.time_access;
		de->CTime=data.time_create;
		de->MTime=data.time_write;

		if (de->Attrib&CPPLDIR_DIR) de->AttrStr[0]='d';
		if (de->Attrib&CPPLDIR_LINK) de->AttrStr[0]='l';
		Files.Add(de);
		// Nächster Datensatz
		if (_wfindnexti64(handle,&data)<0) break;
	}
	_findclose(handle);
	Resort(sort);
	return 1;
#elif defined HAVE_OPENDIR
	clear();
	sort=s;
	Path=path;
	Path.trim();
	Path.trimRight("/");
	Path.trimRight("\\");
	DIR *dir=opendir((const char*)Path.toLocalEncoding());
	if (!dir) {
		File::throwErrno(errno,path);
	}
	struct dirent d;
	struct dirent *result;
	DirEntry de;
	String CurrentFile;
	while (1==1) {
		if (readdir_r(dir,&d,&result)!=0) {
			int e=errno;
			closedir(dir);
			File::throwErrno(e,path);
		}
		if (result==NULL) break;
		CurrentFile=Path+"/"+String(d.d_name);
		File::stat(CurrentFile,de);
		Files.add(de);
	}
	closedir(dir);
	resort(sort);
#else
	throw UnsupportedFeatureException("Dir::open");
#endif
}


/*!\brief Verzeichnis-Eintrag auf STDOUT ausgeben
 *
 * Mit dieser Funktion kann ein Verzeichniseintrag auf STDOUT ausgegeben werden.
 * Die Ausgabe ist ähnlich der des "ls"-Befehls unter Unix, enthält jedoch
 * nicht die Benutzerrechte. Die Funktion wurde hauptsächlich zu Debuggingzwecken
 * eingebaut.
 *
 * \param[in] de Pointer auf einen Verzeichniseintrag
 */
void Dir::print(const DirEntry &de) const
{
	printf ("%ls %3i ",(const wchar_t*)de.AttrStr,de.NumLinks);
	printf ("%5i %5i ",de.Uid, de.Gid);
	printf ("%10llu ",de.Size);
	printf ("%ls %ls\n",(const wchar_t*)de.MTime.get(),(const wchar_t*)de.Filename);
}


/*!\brief Verzeichnis auf STDOUT ausgeben
 *
 * Mit dieser Funktion wird das mit CDir::Open ausgewählte Verzeichnis
 * auf STDOUT ausgegeben.
 */
void Dir::print() const
{
	ppl7::List<const DirEntry*>::Iterator it;
	const DirEntry *de;
	printf ("Directory Listing: %ls\n",(const wchar_t*)Path);
	printf ("Total Files: %zu\n",num());
	SortedFiles.reset(it);
	try {
		while ((de=SortedFiles.getNext(it))) {
			print(*de);
		}
	} catch (EndOfListException) {

	}

}


/*!\brief Anzahl Dateien
 *
 * \desc
 * Diese Funktion liefert die Anzahl Dateien im geöffneten Verzeichnis zurück. Sie
 * gibt daher erst nach Aufruf von CDir::Open einen korrekten Wert zurück.
 * @return
 */
size_t Dir::num() const
{
	return Files.count();
}

/*!\brief Anzahl Dateien
 *
 * \desc
 * Diese Funktion liefert die Anzahl Dateien im geöffneten Verzeichnis zurück. Sie
 * gibt daher erst nach Aufruf von CDir::Open einen korrekten Wert zurück.
 * @return
 */
size_t Dir::count() const
{
	return Files.count();
}


/*!\brief Sortierung ändern
 *
 * \desc
 * Durch Aufruf dieser Funktion kann die Sortierreihenfolge für die Get...-Befehle
 * geändert werden. Standardmäßig werden die Dateien unsortiert zurückgegeben.
 * Die Reihenfolge hängt somit im Wesentlichen davon ab, in welcher Reihenfolge
 * die Dateien erstellt wurden, aber auch von Betriebs- und Filesystemabhängigen
 * Vorgängen.
 * \par
 * Die Sortierreihenfolge läßt sich jedoch jederzeit ändern, auch nach Aufruf
 * der CDir::Open Funktion.
 * \param[in] s Die gewünschte Sortierreihenfolge. Siehe dazu auch CDir::Sort
 * \note
 * Durch Aufruf dieser Funktion wird der interne Zeiger für die Get-Funktionen
 * wieder auf den ersten Eintrag des Verzeichnisses zurückgesetzt.
 */

void Dir::resort(Sort s)
{
	SortedFiles.clear();
	switch (s) {
		case SORT_NONE: resortNone(); break;
		case SORT_FILENAME: resortFilename(); break;
		case SORT_FILENAME_IGNORCASE: resortFilenameIgnoreCase(); break;
		case SORT_ATIME: resortATime(); break;
		case SORT_CTIME: resortCTime(); break;
		case SORT_MTIME: resortMTime(); break;
		case SORT_SIZE: resortSize(); break;
		default: throw IllegalArgumentException();
	}
	sort=s;
}

void Dir::resortNone()
{
	ppl7::List<DirEntry>::Iterator it;
	Files.reset(it);
	try {
		while (1) {
			const DirEntry &de=Files.getNext(it);
			SortedFiles.add(&de);
		}
	} catch (EndOfListException) {

	}
	return;
}

void Dir::resortFilename()
{
	ppl7::List<DirEntry>::Iterator it;
	Files.reset(it);

	AVLTree<String, const DirEntry*> sorter;
	sorter.allowDupes(true);
	String filename;
	try {
		while (1) {
			const DirEntry &de=Files.getNext(it);
			sorter.add(de.Filename,&de);
		}
	} catch (EndOfListException) {

	}
	AVLTree<String, const DirEntry*>::Iterator sortit;
	sorter.reset(sortit);
	while (sorter.getNext(sortit)) {
		SortedFiles.add(sortit.value());
	}
}

void Dir::resortFilenameIgnoreCase()
{
	ppl7::List<DirEntry>::Iterator it;
	Files.reset(it);

	AVLTree<String, const DirEntry*> sorter;
	sorter.allowDupes(true);
	String filename;
	try {
		while (1) {
			const DirEntry &de=Files.getNext(it);
			filename.set(de.Filename);
			filename.lowerCase();
			sorter.add(filename,&de);
		}
	} catch (EndOfListException) {

	}
	AVLTree<String, const DirEntry*>::Iterator sortit;
	sorter.reset(sortit);
	while (sorter.getNext(sortit)) {
		SortedFiles.add(sortit.value());
	}
}

void Dir::resortMTime()
{
	ppl7::List<DirEntry>::Iterator it;
	AVLTree<DateTime, const DirEntry*> sorter;
	sorter.allowDupes(true);
	Files.reset(it);
	try {
		while (1) {
			const DirEntry &de=Files.getNext(it);
			sorter.add(de.MTime,&de);
		}
	} catch (EndOfListException) {

	}
	AVLTree<DateTime, const DirEntry*>::Iterator sortit;
	sorter.reset(sortit);
	while (sorter.getNext(sortit)) {
		SortedFiles.add(sortit.value());
	}

}

void Dir::resortCTime()
{
	ppl7::List<DirEntry>::Iterator it;
	AVLTree<DateTime, const DirEntry*> sorter;
	sorter.allowDupes(true);
	Files.reset(it);
	try {
		while (1) {
			const DirEntry &de=Files.getNext(it);
			sorter.add(de.CTime,&de);
		}
	} catch (EndOfListException) {

	}
	AVLTree<DateTime, const DirEntry*>::Iterator sortit;
	sorter.reset(sortit);
	while (sorter.getNext(sortit)) {
		SortedFiles.add(sortit.value());
	}
	return;
}

void Dir::resortATime()
{
	ppl7::List<DirEntry>::Iterator it;
	AVLTree<DateTime, const DirEntry*> sorter;
	sorter.allowDupes(true);
	Files.reset(it);
	try {
		while (1) {
			const DirEntry &de=Files.getNext(it);
			sorter.add(de.ATime,&de);
		}
	} catch (EndOfListException) {

	}
	AVLTree<DateTime, const DirEntry*>::Iterator sortit;
	sorter.reset(sortit);
	while (sorter.getNext(sortit)) {
		SortedFiles.add(sortit.value());
	}
	return;
}

void Dir::resortSize()
{
	ppl7::List<DirEntry>::Iterator it;
	AVLTree<ppluint64, const DirEntry*> sorter;
	sorter.allowDupes(true);
	Files.reset(it);
	try {
		while (1) {
			const DirEntry &de=Files.getNext(it);
			sorter.add(de.Size,&de);
		}
	} catch (EndOfListException) {

	}
	AVLTree<ppluint64, const DirEntry*>::Iterator sortit;
	sorter.reset(sortit);
	while (sorter.getNext(sortit)) {
		SortedFiles.add(sortit.value());
	}
}

/*!\brief Zeiger auf den ersten Eintrag des Verzeichnisses
 *
 * \desc
 * Mit dieser Funktion wird der Zeiger des Iterators \p it auf den ersten Eintrag
 * im Verzeichnis gesetzt. Der nächste Aufruf von einer der "getNext..."-Funktionen
 * würde somit den ersten Eintrag zurückliefern.
 * \param it Iterator vom Typ ppl7::Dir::Iterator
 */
void Dir::reset(Iterator &it) const
{
	SortedFiles.reset(it);
}

/*!\brief Erster Verzeichniseintrag
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der interne Zeiger auf die erste gefundene Datei gesetzt und ein Pointer
 * auf dessen CDirEntry-Struktur zurückgegeben. Alle weiteren Dateien können mit CDir::GetNext ausgelesen werden.
 * \par
 * Die Reihenfolge der durch Dir::getFirst und Dir::getNext zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort und Dir::open
 * \param it Iterator vom Typ ppl7::Dir::Iterator
 * @return Referenz auf die erste Datei des Verzeichnisses.
 */
const DirEntry &Dir::getFirst(Iterator &it) const
{
	return *SortedFiles.getFirst(it);
}

/*!\brief Nächster Verzeichniseintrag
 *
 * \desc
 * Diese Funktion liefert die nächste Datei aus dem geöffneten Verzeichnis zurück.
 * \par
 * Die Reihenfolge der durch Dir::getFirst und Dir::getNext zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort und Dir::open
 *
 * \param it Iterator vom Typ ppl7::Dir::Iterator
 * @return Referenz auf die nächste Datei des Verzeichnisses.
 */
const DirEntry &Dir::getNext(Iterator &it) const
{
	return *SortedFiles.getNext(it);
}

/*!\brief Erster Verzeichniseintrag, der zu einem bestimmten Muster passt
 *
 * \desc
 * Durch Aufruf dieser Funktion wird die erste Datei aus dem Verzeichnis zurückgeliefert, die
 * zu dem angegebenen Muster \p pattern passt. Alle weiteren Dateien können mit CDir::GetNextPattern
 * ausgelesen werden.
 * \par
 * Die Reihenfolge der durch CDir::GetFirstPattern und CDir::GetNextPattern zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu CDir::Sort, CDir::Resort und CDir::Open
 *
 * \param[in] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] pattern Ein beliebiges Suchpattern, wie es auch beim Unix-Befehl "ls" oder mit
 * "dir" unter Windows angegeben werden kann. Dabei sind die Wildcards "*" und "?" erlaubt.
 * Das Sternchen "*" steht dabei für beliebig viele Zeichen, das Fragezeichen "?" für ein einzelnes.
 * \param[in] ignorecase Wird diese Variable auf "true" gesetzt, wird Groß- und Kleinschreibung
 * ignoriert. Wird als Pattern beispielsweise "*.TXT" angegeben, würde auch "*.txt" passen.
 * Der Default ist "false"
 *
 * @return Pointer auf die erste Datei des Verzeichnisses, die zum angegebenen Muster passt, oder
 * NULL wenn keine Datei passt, das Verzeichnis leer ist oder kein
 * gültiges Verzeichnis ausgewählt wurde.
 */
const DirEntry &Dir::getFirstPattern(Iterator &it, const String &pattern, bool ignorecase) const
{
	reset(it);
	return getNextPattern(it, pattern, ignorecase);
}

/*!\brief Nächster Verzeichniseintrag, der zu einem bestimmten Muster passt
 *
 * \desc
 * Diese Funktion liefert die nächste Datei aus dem geöffneten Verzeichnis zurück, die
  * zu dem angegebenen Muster \p pattern passt.
 * \par
 * Die Reihenfolge der durch CDir::GetFirstPattern und CDir::GetNextPattern zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu CDir::Sort, CDir::Resort und CDir::Open
 *
 * \param[in] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] pattern Ein beliebiges Suchpattern, wie es auch beim Unix-Befehl "ls" oder mit
 * "dir" unter Windows angegeben werden kann. Dabei sind die Wildcards "*" und "?" erlaubt.
 * Das Sternchen "*" steht dabei für beliebig viele Zeichen, das Fragezeichen "?" für ein einzelnes.
 * \param[in] ignorecase Wird diese Variable auf "true" gesetzt, wird Groß- und Kleinschreibung
 * ignoriert. Wird als Pattern beispielsweise "*.TXT" angegeben, würde auch "*.txt" passen.
 * Der Default ist "false"
 *
 * @return Pointer auf die nächste Datei des Verzeichnisses, oder NULL, wenn keine weiteren Dateien
 * vorhanden sind, die auf das Muster passen, das Verzeichnis leer ist oder kein gültiges
 * Verzeichnis ausgewählt wurde.
 */
const DirEntry &Dir::getNextPattern(Iterator &it, const String &pattern, bool ignorecase) const
{
	const DirEntry *de;
	String Pattern;
	Pattern=pattern;
	//printf ("Pattern: %ls\n",(const wchar_t*)Pattern);
	Pattern.pregEscape();
	//printf ("Pattern: %ls\n",(const wchar_t*)Pattern);
	Pattern.replace("\\*",".*");
	Pattern.replace("\\?",".");
	Pattern="/^"+Pattern;
	Pattern+="$/";
	if (ignorecase) Pattern+="i";
	//printf ("Pattern: %ls\n",(const wchar_t*)Pattern);
	while (1) {
		de=SortedFiles.getNext(it);
		if (!de) throw EndOfListException();
		// Patternmatch
		//printf ("Match gegen: %ls\n",(const wchar_t*)Name);
		if (de->Filename.pregMatch(Pattern)) return *de;
	}
}

/*!\brief Erster Verzeichniseintrag, der zu der angegebenen Regular Expression passt
 *
 * \desc
 * Durch Aufruf dieser Funktion wird die erste Datei aus dem Verzeichnis zurückgeliefert, die
 * zu der angegebenen Regular Expression \p regexp passt. Alle weiteren Dateien können mit
 * CDir::GetNextRegExp ausgelesen werden.
 * \par
 * Die Reihenfolge der durch CDir::GetFirstRegExp und CDir::GetNextRegExp zurückgelieferten
 * Dateien hängt von der eingestellten Sortierung ab. Siehe dazu CDir::Sort, CDir::Resort
 * und CDir::Open
 *
 * \param[in] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] regexp Eine beliebige Perl kompatible Regular Expression. Beispiel:"/^*.txt$/i"
 *
 * @return Pointer auf die erste Datei des Verzeichnisses, oder NULL wenn das Verzeichnis leer ist,
 * kein Dateiname auf die angegebene Regular Expression passt oder kein
 * gültiges Verzeichnis ausgewählt wurde.
 */
const DirEntry &Dir::getFirstRegExp(Iterator &it, const String &regexp) const
{
	reset(it);
	return getNextRegExp(it,regexp);
}

/*!\brief Nächster Verzeichniseintrag, der zu der angegebenen Regular Expression passt
 *
 * \desc
 * Diese Funktion liefert die nächste Datei aus dem geöffneten Verzeichnis zurück, die
 * zu der angegebenen Regular Expression \p regexp passt.
 *
 * \par
 * Die Reihenfolge der durch CDir::GetFirstRegExp und CDir::GetNextRegExp zurückgelieferten
 * Dateien hängt von der eingestellten Sortierung ab. Siehe dazu CDir::Sort, CDir::Resort
 * und CDir::Open
 *
 * \param[in] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] regexp Eine beliebige Perl kompatible Regular Expression. Beispiel:"/^*.txt$/i"
 *
 * @return Pointer auf die nächste Datei des Verzeichnisses, oder NULL wenn das Verzeichnis leer ist,
 * kein weiterer Dateiname auf die angegebene Regular Expression passt oder kein
 * gültiges Verzeichnis ausgewählt wurde.
 */
const DirEntry &Dir::getNextRegExp(Iterator &it, const String &regexp) const
{
	const DirEntry *de;
	while (1) {
		de=SortedFiles.getNext(it);
		if (!de) throw EndOfListException();
		// Patternmatch
		if (de->Filename.pregMatch(regexp)) return *de;
	}
}



} // EOF namespace ppl7
