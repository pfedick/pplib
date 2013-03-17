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


#ifndef _PPL7_INCLUDE
#define _PPL7_INCLUDE

#define PPL_VERSION_MAJOR	7
#define PPL_VERSION_MINOR	0
#define PPL_VERSION_BUILD	0
#define PPL_RELEASEDATE		20111230
#define PPL_COPYRIGHT		"Copyright (c) 2011 by Patrick Fedick"

// Inlcude PPL7 configuration file
#ifndef _PPL7_CONFIG
	#ifdef PPL7LIB
		#ifdef MINGW32
			#include "config.h"
		#elif defined _WIN32
			#include "ppl7-config.h"
		#else
			#include "config.h"
		#endif
	#else
		#include <ppl7-config.h>
	#endif
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef  HAVE_STRINGS_H
#include <strings.h>
#endif

#include <string>

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef WITH_QT
	#include <QString>
	#include <QVariant>
	#include <QByteArray>
#endif


#ifdef putc
#undef putc
#endif

#ifdef putwc
#undef putwc
#endif

#ifdef puts
#undef puts
#endif

#ifdef putws
#undef putws
#endif

#ifdef getc
#undef getc
#endif

#ifdef getwc
#undef getwc
#endif

#ifdef gets
#undef gets
#endif

#ifdef getws
#undef getws
#endif


// Exceptions
#ifdef PPL7LIB
	#include "ppl7-exceptions.h"
#else
	#include <ppl7-exceptions.h>
#endif

//#ifndef MINGW32
	#ifdef _WIN32
		#define bzero ZeroMemory
		//#define bcopy CopyMemory   // CopyMemory hat andere Parameter-Reihenfolge als bcopy!
		#define strcasecmp stricmp
		#define mode_t ppluint32
	#endif
//#endif


namespace ppl7 {

 class MemoryHeap
 {
 	private:
 		void		*blocks;
 		size_t		myElementSize, increaseSize;
 		size_t		myGrowPercent;
 		size_t		blocksAllocated, blocksUsed;
 		size_t		mem_allocated;
 		size_t		mem_used;
 		size_t		freeCount;

 		void		increase(size_t num);

 	public:
 		PPLNORMALEXCEPTION(NotInitializedException);
 		PPLNORMALEXCEPTION(AlreadyInitializedException);
 		PPLNORMALEXCEPTION(HeapCorruptedException);
 		PPLNORMALEXCEPTION(ElementNotInHeapException);



 		MemoryHeap();
 		MemoryHeap(size_t elementsize, size_t startnum, size_t increase, size_t growpercent=30);
 		~MemoryHeap();
 		void clear();
 		void init(size_t elementsize, size_t startnum, size_t increase, size_t growpercent=30);
 		void *malloc();
 		void *calloc();
 		void free(void *element);
 		size_t memoryUsed() const;
 		size_t memoryAllocated() const;
 		void dump() const;
 		size_t capacity() const;
 		size_t count() const;
 		size_t elementSize() const;
 		void reserve(size_t num);
 		void cleanup();
 };

class MemoryGroup
{
	private:
		 void *first, *last;
		 size_t totalSize;
		 size_t totalBlocks;

		 void addToList(void *block);
		 void removeFromList(void *block);
	public:
		 MemoryGroup();
		 ~MemoryGroup();
		 void clear();
		 void *malloc(size_t size);
		 void *calloc(size_t size);
		 void *realloc(void *adr, size_t size);
		 void free(void *adr);
		 char *strdup(const char *string);
		 char *strndup(const char *string, size_t size);
		 size_t count() const;
		 size_t size() const;
};


}
 // Inlcude PPL7 Algorithms
 #ifdef PPL7LIB
 	#include "ppl7-algorithms.h"
 #else
 	#include <ppl7-algorithms.h>
 #endif


// Inlcude PPL7 Data-Types
#ifdef PPL7LIB
	#include "ppl7-types.h"
#else
	#include <ppl7-types.h>
#endif





namespace ppl7 {

// Functions
void PrintDebug(const char *format, ...);
void PrintDebugTime(const char *format, ...);
void SetGlobalOutput(int type);


void HexDump(const void *address, size_t bytes, bool skipheader);
void HexDump(const void *address, size_t bytes);
String ToBase64(const ByteArrayPtr &bin);
ByteArray FromBase64(const String &str);
ppluint32 Crc32(const void* buffer, size_t size);
String Md5(const void* buffer, size_t size);

String StripSlashes(const String &str);
String EscapeHTMLTags(const String &html);
String UnescapeHTMLTags(const String &html);
ByteArray Hex2ByteArray(const String &hex);
String ToHex(const ByteArrayPtr &bin);
String UrlEncode(const String &text);
String UrlDecode(const String &text);


String Trim(const String &str);
String UpperCase(const String &str);
String LowerCase(const String &str);
int StrCmp(const String &s1, const String &s2);
int StrCaseCmp(const String &s1, const String &s2);
ssize_t Instr (const String &haystack, const String &needle, size_t start=0);
ssize_t InstrCase (const String &haystack, const String &needle, size_t start=0);
ssize_t Instr (const char * haystack, const char * needle, size_t start=0);
ssize_t Instrcase (const char * haystack, const char * needle, size_t start=0);
ssize_t Instr (const wchar_t * haystack, const wchar_t * needle, size_t start=0);
ssize_t Instrcase (const wchar_t * haystack, const wchar_t * needle, size_t start=0);

String Left(const String &str, size_t num);
String Right(const String &str, size_t num);
String Mid(const String &str, size_t start, size_t num=(size_t)-1);
String SubStr(const String &str, size_t start, size_t num=(size_t)-1);
String ToString(const char *fmt, ...);
String Replace(const String &string, const String &search, const String &replace);
bool IsTrue(const String &str);
Array StrTok(const String &string, const String &div);
void StrTok(Array &result, const String &string, const String &div);

const char * GetArgv (int argc, char * argv[], const char * argument);

bool PregMatch(const String &expression, const String &subject);
bool PregMatch(const String &expression, const String &subject, Array &matches, size_t maxmatches=16);

Array Sort(const Array &array, bool unique=false);
Array SortReverse(const Array &array, bool unique=false);
size_t rand(size_t min, size_t max);


// Speicherzugriff
void Poke8 (void *Adresse, ppluint32 Wert);
void Poke16 (void *Adresse, ppluint32 Wert);
void Poke24 (void *Adresse, ppluint32 Wert);
void Poke32 (void *Adresse, ppluint32 Wert);
void Poke64 (void *Adresse, ppluint64 Wert);
void PokeFloat(void *Adresse, float Wert);
ppluint32 Peek8 (const void *Adresse);
ppluint32 Peek16 (const void *Adresse);
ppluint32 Peek24 (const void *Adresse);
ppluint32 Peek32 (const void *Adresse);
ppluint64 Peek64 (const void *Adresse);
float	PeekFloat(const void *Adresse);

// Network-Byte-Order
void PokeN8 (void *Adresse, ppluint32 Wert);
void PokeN16 (void *Adresse, ppluint32 Wert);
void PokeN32 (void *Adresse, ppluint32 Wert);
void PokeN64 (void *Adresse, ppluint64 Wert);

ppluint32 PeekN8 (const void *Adresse);
ppluint32 PeekN16 (const void *Adresse);
ppluint32 PeekN32 (const void *Adresse);
ppluint64 PeekN64 (const void *Adresse);


// cpu.cpp
namespace CPUCAPS {
	enum {
		NONE			= 0x00000000,
		HAVE_CPUID		= 0x00000001,
		HAVE_MMX		= 0x00000002,
		HAVE_MMXExt		= 0x00000004,
		HAVE_3DNow		= 0x00000008,
		HAVE_3DNowExt	= 0x00000010,
		HAVE_SSE		= 0x00000020,
		HAVE_SSE2		= 0x00000040,
		HAVE_AMD64		= 0x00000080,
		HAVE_SSE3		= 0x00000100,
		HAVE_SSSE3		= 0x00000200,
		HAVE_SSE4a		= 0x00000400,
		HAVE_SSE41		= 0x00000800,
		HAVE_SSE42		= 0x00001000,
		HAVE_AES		= 0x00002000,
	};
}

typedef struct {
	ppluint32	caps;							// Struktur kann um weitere Informationen erweitert werden
	ppluint32	bits;

} CPUCaps;

ppluint32 GetCPUCaps(CPUCaps &cpu);
ppluint32 GetCPUCaps();

// Time
ppl_time_t GetTime(PPLTIME *t);
ppl_time_t GetTime(PPLTIME *t, ppl_time_t tt);
ppl_time_t GetTime(PPLTIME &t, ppl_time_t tt);
ppl_time_t GetTime();
int USleep(ppluint64 microseconds);		// 1 sec = 1000000 microseconds
int MSleep(ppluint64 milliseconds);		// 1 sec = 1000 milliseconds
int SSleep(ppluint64 seconds);
double GetMicrotime();
ppluint64 GetMilliSeconds();

//void datumsauswertung (pplchar * d, pplchar * dat);
/*
ppluint64 Datum2Sekunden(char *datum, char *zeit);
ppluint64 GetUTC(char *datum, char *zeit);
const char * UTC2Date(ppluint64 sec, char *datum, char *uhrzeit);
ppldd Sekunden2Datum(ppluint64 sec, ppldd *datum, ppldd *uhrzeit);
ppluint64 MkTime(const char *year, const char *month, const char *day, const char *hour=NULL, const char *min=NULL, const char *sec=NULL);
ppluint64 MkTime(int year, int month, int day, int hour=0, int min=0, int sec=0);
ppluint64 MkTime(String *year, String *month, String *day, String *hour, String *min, String *sec);
ppluint64 MkTime(const String &year, const String &month, const String &day, const String &hour="0", const String &min="0", const String &sec="0");
ppluint64 MkTime(const char *iso8601date, PPLTIME *t=NULL);
ppluint64 MkTime(String *iso8601date, PPLTIME *t=NULL);

const char *MkISO8601Date (String &buffer, ppluint64 sec);
const char *MkISO8601Date (String &buffer, PPLTIME *t=NULL);
char *MkISO8601Date (char *buffer, size_t size, ppluint64 sec);
char *MkISO8601Date (char *buffer, size_t size, PPLTIME *t=NULL);
String MkISO8601Date (ppluint64 sec);
String MkISO8601Date (PPLTIME *t=NULL);

String MkRFC822Date (PPLTIME &t);
String MkRFC822Date (ppluint64 sec=0);

const char *MkDate (String &buffer, const char *format, ppluint64 sec);
String MkDate(const char *format, ppluint64 sec);
String Long2Date(const char *format, int value);
char *MkDate (char *buffer, int size, const char *format, ppluint64 sec);
*/

//! \brief Timer-Klasse
class Timer
{
	private:
		double startzeit, endzeit, myduration;
	public:
		Timer();
		~Timer();
		double start();
		double stop();
		double currentDuration();
		double duration();
};



//! \brief Synchronisation von Threads
class Mutex
{
	private:
		void *handle;

	public:
		Mutex() throw(OutOfMemoryException);
		~Mutex() throw();
		int lock() throw();
		int unlock() throw();
		int tryLock() throw();
		int wait(int milliseconds=0) throw();
		int signal() throw();
};


// Threads
ppluint64 ThreadID();
void	*GetTLSData();
void	SetTLSData(void *data);
ppluint64	StartThread(void (*start_routine)(void *),void *data=NULL);

class Thread
{
	private:
		Mutex	threadmutex;
		void *threaddata;
		int flags;
		int IsRunning;
		int IsSuspended;
		int deleteMe;
		int myPriority;

	public:

		enum Priority {
			UNKNOWN=0,
			LOWEST,
			BELOW_NORMAL,
			NORMAL,
			ABOVE_NORMAL,
			HIGHEST
		};

		Thread();
		virtual ~Thread();
		void	threadSuspend();
		void	threadResume();
		void	threadStop();
		void	threadSignalStop();
		void	threadStart();
		void	threadStartUp();
		int		threadIsRunning();
		int		threadIsSuspended();
		int		threadGetFlags();
		ppluint64  threadGetID();
		int		threadShouldStop();
		void	threadWaitSuspended(int msec=0);
		void	threadDeleteOnExit(int flag=1);
		int		threadShouldDeleteOnExit();
		int		threadSetPriority(int priority);
		int		threadGetPriority();
		void	threadIdle();
		int		threadSetStackSize(size_t size=0);
		size_t	threadGetStackSize();
		size_t	threadGetMinimumStackSize();

		virtual void run();
};


void ThreadSetPriority(Thread::Priority priority);
Thread::Priority ThreadGetPriority();



class FileAttr {
	public:
		enum Attributes {
			IFFILE		= 0x10000,
			IFSOCK		= 0x20000,
			IFDIR		= 0x40000,
			IFLINK		= 0x80000,
			ISUID		= 0x4000,
			ISGID		= 0x2000,
			ISVTX		= 0x1000,
			STICKY		= 0x1000,
			USR_READ	= 0x0400,
			USR_WRITE	= 0x0200,
			USR_EXECUTE	= 0x0100,
			GRP_READ	= 0x0040,
			GRP_WRITE	= 0x0020,
			GRP_EXECUTE	= 0x0010,
			OTH_READ	= 0x0004,
			OTH_WRITE	= 0x0002,
			OTH_EXECUTE	= 0x0001,
			CHMOD_755	= 0x0755,
			CHMOD_644	= 0x0644,
			NONE		= 0
		};
};

class DirEntry;



class FileObject
{
	private:
		String MyFilename;

	protected:
		char * buffer;

	public:
		enum SeekOrigin {
			SEEKCUR=1,
			SEEKEND,
			SEEKSET
		};
		//! @name Exceptions
		//@{
		PPLNORMALEXCEPTION(FileNotOpenException);
		PPLNORMALEXCEPTION(FileSeekException);
		PPLNORMALEXCEPTION(ReadException);
		PPLNORMALEXCEPTION(WriteException);
		PPLNORMALEXCEPTION(EndOfFileException);
		PPLPARAMETERISEDEXCEPTION(FileOpenException);
		PPLPARAMETERISEDEXCEPTION(FileNotFoundException);				// ENOENT
		PPLNORMALEXCEPTION(InvalidArgumentsException);					// EINVAL
		PPLPARAMETERISEDEXCEPTION(InvalidFileNameException);			// ENOTDIR, ENAMETOOLONG, ELOOP
		PPLPARAMETERISEDEXCEPTION(PermissionDeniedException);			// EACCESS, EPERM
		PPLPARAMETERISEDEXCEPTION(ReadOnlyException);					// EROFS
		PPLPARAMETERISEDEXCEPTION(NoRegularFileException);				// EISDIR
		PPLNORMALEXCEPTION(TooManyOpenFilesException);					// EMFILE
		PPLPARAMETERISEDEXCEPTION(UnsupportedFileOperationException);	// EOPNOTSUPP
		PPLPARAMETERISEDEXCEPTION(TooManySymbolicLinksException);		// ELOOP
		PPLNORMALEXCEPTION(FilesystemFullException);					// ENOSPC
		PPLNORMALEXCEPTION(QuotaExceededException);						// EDQUOT
		PPLNORMALEXCEPTION(IOErrorException);							// EIO
		PPLNORMALEXCEPTION(BadFiledescriptorException);					// EABDF
		PPLNORMALEXCEPTION(BadAddressException);						// EFAULT
		PPLNORMALEXCEPTION(OverflowException);							// EOVERFLOW
		PPLNORMALEXCEPTION(FileExistsException);						// EEXIST
		PPLNORMALEXCEPTION(OperationBlockedException);					// EAGAIN
		PPLNORMALEXCEPTION(DeadlockException);							// EDEADLK
		PPLNORMALEXCEPTION(OperationInterruptedException);				// EINTR
		PPLNORMALEXCEPTION(TooManyLocksException);						// ENOLCK
		PPLNORMALEXCEPTION(IllegalOperationOnPipeException);			// ESPIPE
		PPLNORMALEXCEPTION(BufferExceedsLimitException);

		//@}

		FileObject();
		virtual ~FileObject();

		void				setFilename(const char *filename);
		void				setFilename(const String &filename);
		const String&		filename() const;

		size_t				write (const void * source, size_t bytes, ppluint64 fileposition);
		size_t				write (const void * source, size_t bytes);
		size_t				write (const ByteArrayPtr &object, size_t bytes=0);
		size_t				read (void * target, size_t bytes, ppluint64 fileposition);
		size_t				read (void * target, size_t bytes);
		size_t				read (ByteArray &target, size_t bytes);
		ppluint64			copy (FileObject &quellfile, ppluint64 quelloffset, ppluint64 bytes, ppluint64 zieloffset);
		ppluint64			copy (FileObject &quellfile, ppluint64 bytes);
		int					gets (String &buffer, size_t num=1024);
		int					getws (String &buffer, size_t num=1024);
		String				gets (size_t num=1024);
		String				getws (size_t num=1024);
		void				putsf (const char *fmt, ... );
		void				puts (const String &str);
		void				putws (const WideString &str);
		const char			*map();
		char				*load();
		int					load(ByteArray &target);
		String				md5();

		// Virtuelle Funktionen
		virtual void		close ();
		virtual ppluint64	seek (ppluint64 position);
		virtual	ppluint64	seek (pplint64 offset, SeekOrigin origin);
		virtual ppluint64	tell();
		virtual void		rewind();
		virtual size_t		fread(void * ptr, size_t size, size_t nmemb);
		virtual size_t		fwrite(const void * ptr, size_t size, size_t nmemb);
		virtual char *		fgets (char *buffer, size_t num);
		virtual wchar_t*	fgetws (wchar_t *buffer, size_t num=1024);
		virtual void		fputs (const char *str);
		virtual void		fputws (const wchar_t *str);
		virtual	void		fputc (int c);
		virtual	int			fgetc();
		virtual	void		fputwc (wchar_t c);
		virtual	wchar_t		fgetwc();
		virtual bool		eof() const;
		virtual ppluint64	size() const;
		virtual const char	*map(ppluint64 position, size_t size);
		virtual char		*mapRW(ppluint64 position, size_t size);
		virtual	void		unmap();
		virtual void		setMapReadAhead(size_t bytes);
		virtual int			getFileNo() const;
		virtual void		flush();
		virtual void		sync();
		virtual void		truncate(ppluint64 length);
		virtual bool		isOpen() const;
		virtual void		lockShared(bool block=true);
		virtual void		lockExclusive(bool block=true);
		virtual void		unlock();
};


class MemFile : public FileObject
{
	private:
		size_t	mysize;
		size_t	pos;
		size_t	maxsize;
		size_t	buffersize;
		char * MemBase;
		bool	readonly;

		void resizeBuffer(size_t size);
		void increaseBuffer(size_t bytes);

	public:

		MemFile ();
		MemFile (void * adresse, size_t size, bool writeable=false);
		MemFile (const ByteArrayPtr &memory);
		~MemFile();

		void		open(void * adresse, size_t size, bool writeable=false);
		void		open(const ByteArrayPtr &memory);
		void		openReadWrite(void * adresse, size_t size);
		char		*adr(size_t adresse);
		void		setMaxSize(size_t size);
		size_t		maxSize() const;

		// Virtuelle Funktionen
		virtual void		close();
		virtual void		rewind();
		virtual ppluint64	seek (ppluint64 position);
		virtual	ppluint64	seek (pplint64 offset, SeekOrigin origin);
		virtual ppluint64	tell();
		virtual size_t		fread(void * ptr, size_t size, size_t nmemb);
		virtual size_t		fwrite(const void * ptr, size_t size, size_t nmemb);
		virtual char *		fgets (char *buffer, size_t num);
		virtual wchar_t*	fgetws (wchar_t *buffer, size_t num=1024);
		virtual	void		fputc (int c);
		virtual	int			fgetc();
		virtual	void		fputwc (wchar_t c);
		virtual	wchar_t		fgetwc();
		virtual void		fputs (const char *str);
		virtual void		fputws (const wchar_t *str);
		virtual bool		eof() const;
		virtual ppluint64	size() const;
		virtual const char	*map(ppluint64 position, size_t size);
		virtual char		*mapRW(ppluint64 position, size_t size);
		virtual	void		unmap();
		virtual void		setMapReadAhead(size_t bytes);
		virtual int			getFileNo() const;
		virtual void		flush();
		virtual void		sync();
		virtual void		truncate(ppluint64 length);
		virtual bool		isOpen() const;
		virtual void		lockShared(bool block=true);
		virtual void		lockExclusive(bool block=true);
		virtual void		unlock();
};

class File : public FileObject
{
	private:
		void * ff;
		char * MapBase;
		ppluint64	LastMapStart;
		ppluint64	LastMapSize;
		int			LastMapProtection;
		ppluint64	ReadAhead;
		ppluint64	mysize;
		ppluint64	pos;
		bool isPopen;

		int munmap(void *addr, size_t len);
		void *mmap(ppluint64 position, size_t size, int prot, int flags);



	public:
		PPLNORMALEXCEPTION(IllegalFilemodeException);
		enum FileMode {
			READ=1,
			WRITE,
			READWRITE,
			APPEND,
		};
	private:
		const char *fmode(FileMode mode);
		const char *fmodepopen(FileMode mode);
		void throwErrno(int e);

	public:
		static void throwErrno(int e, const String &filename);

		File ();
		File (const String &filename, FileMode mode=READ);
		File (FILE * handle);
		virtual ~File();

		void		open (const String &filename, FileMode mode=READ);
		void		open (const char * filename, FileMode mode=READ);
		void		open (FILE * handle);
		void		openTemp(const String &filetemplate);
		void		openTemp(const char *filetemplate);
		void		popen(const char *command, FileMode mode=READ);
		void		popen(const String &command, FileMode mode=READ);
		void		erase();

		// Virtuelle Funktionen
		virtual void		close ();
		virtual void		rewind();
		virtual ppluint64	seek (ppluint64 position);
		virtual	ppluint64	seek (pplint64 offset, SeekOrigin origin);
		virtual ppluint64	tell();
		virtual ppluint64	size() const;
		virtual bool		isOpen() const;
		virtual size_t		fread(void * ptr, size_t size, size_t nmemb);
		virtual size_t		fwrite(const void * ptr, size_t size, size_t nmemb);
		virtual char *		fgets (char *buffer, size_t num);
		virtual wchar_t*	fgetws (wchar_t *buffer, size_t num=1024);
		virtual void		fputs (const char *str);
		virtual void		fputws (const wchar_t *str);
		virtual	void		fputc (int c);
		virtual	int			fgetc();
		virtual	void		fputwc (wchar_t c);
		virtual	wchar_t		fgetwc();
		virtual bool		eof() const;
		virtual int			getFileNo() const;
		virtual void		flush();
		virtual void		sync();
		virtual void		truncate(ppluint64 length);
		virtual void		lockShared(bool block=true);
		virtual void		lockExclusive(bool block=true);
		virtual void		unlock();
		virtual void		setMapReadAhead(size_t bytes);
		virtual const char	*map(ppluint64 position, size_t size);
		virtual char		*mapRW(ppluint64 position, size_t size);
		virtual	void		unmap();


		// Static Functions
		static void load(ByteArray &object, const String &filename);
		static void load(ByteArray &object, const char *filename);
		static void load(String &object, const String &filename);
		static void load(String &object, const char *filename);
		static void *load(const String &filename, size_t *size=NULL);
		static void *load(const char *filename, size_t *size=NULL);
		static void truncate(const String &filename, ppluint64 bytes);
		static void truncate(const char *filename, ppluint64 bytes);
		static bool exists(const String &filename);
		static bool exists(const char *filename);
		static void copy(const String &oldfile, const String &newfile);
		static void move(const String &oldfile, const String &newfile);
		static void rename(const String &oldfile, const String &newfile);
		static void rename(const char *oldfile, const char *newfile);

		static void unlink(const String &filename);
		static void remove(const String &filename);
		static void erase(const String &filename);
		static void touch(const String &filename);
		static void touch(const char *filename);
		static void save(const void *content, size_t size, const String &filename);
		static void save(const void *content, size_t size, const char *filename);
		static void save(const ByteArrayPtr &object, const String &filename);
		static void save(const ByteArrayPtr &object, const char *filename);
		static void chmod(const String &filename, FileAttr::Attributes attr);
		static void chmod(const char *filename, FileAttr::Attributes attr);
		static void stat(const char *filename, DirEntry &result);
		static void stat(const String &filename, DirEntry &result);
		static String getPath(const String &path);
		static String getFilename(const String &path);
};


class DirEntry
{
	public:
		DirEntry();
		DirEntry(const DirEntry& other);
		String		Filename;
		String		Path;
		String		File;
		ppluint64	Size;
		FileAttr::Attributes Attrib;
		ppluint32	Uid;
		ppluint32	Gid;
		ppluint32	Blocks;
		ppluint32	BlockSize;
		ppluint32	NumLinks;
		String		AttrStr;
		DateTime	ATime, CTime, MTime;
		bool		isDir();
		bool		isFile();
		bool		isLink();
		bool		isReadable();
		bool		isWritable();
		bool		isExecutable();
		DirEntry& operator=(const DirEntry& other);
		void		toArray(AssocArray &a) const;
		void		print(const char *label=NULL);
};

class Dir
{
	public:
		enum Sort {
			SORT_NONE,
			SORT_FILENAME,
			SORT_FILENAME_IGNORCASE,
			SORT_ATIME,
			SORT_CTIME,
			SORT_MTIME,
			SORT_SIZE,
		};
	private:
		ppl7::List<DirEntry> Files;
		ppl7::List<const DirEntry*> SortedFiles;
		Sort sort;
		String Path;

		void resortMTime();
		void resortCTime();
		void resortATime();
		void resortSize();
		void resortFilename();
		void resortFilenameIgnoreCase();
		void resortNone();

	public:
		PPLNORMALEXCEPTION(PathnameTooLongException);
		PPLNORMALEXCEPTION(NonexistingPathException);
		PPLNORMALEXCEPTION(PermissionDeniedException);
		PPLNORMALEXCEPTION(CreateDirectoryFailedException);

		typedef ppl7::List<const DirEntry*>::Iterator Iterator;

		Dir();
		Dir(const char *path, Sort s=SORT_NONE);
		Dir(const String &path, Sort s=SORT_NONE);
		~Dir();
		void open(const char *path, Sort s=SORT_NONE);
		void open(const String &path, Sort s=SORT_NONE);
		void resort(Sort s);
		void clear();
		size_t num() const;
		size_t count() const;
		void reset(Iterator &it) const;
		const DirEntry &getFirst(Iterator &it) const;
		const DirEntry &getNext(Iterator &it) const;
		const DirEntry &getFirstPattern(Iterator &it, const String &pattern, bool ignorecase=false) const;
		const DirEntry &getNextPattern(Iterator &it, const String &pattern, bool ignorecase=false) const;
		const DirEntry &getFirstRegExp(Iterator &it, const String &regexp) const;
		const DirEntry &getNextRegExp(Iterator &it, const String &regexp) const;

		bool getFirst(DirEntry &e, Iterator &it) const;
		bool getNext(DirEntry &e, Iterator &it) const;
		bool getFirstPattern(DirEntry &e, Iterator &it, const String &pattern, bool ignorecase=false) const;
		bool getNextPattern(DirEntry &e, Iterator &it, const String &pattern, bool ignorecase=false) const;
		bool getFirstRegExp(DirEntry &e, Iterator &it, const String &regexp) const;
		bool getNextRegExp(DirEntry &e, Iterator &it, const String &regexp) const;

		void print() const;
		void print(const DirEntry &de) const;
		static String currentPath();
		static String homePath();
		static String tempPath();

		static bool exists(const String &dirname);
		static void mkDir(const String &path);
		static void mkDir(const String &path, bool recursive);
		static void mkDir(const String &path, mode_t mode, bool recursive);


};


class Compression
{
	public:
		enum Algorithm {
			Algo_NONE=0,
			Algo_ZLIB,
			Algo_BZIP2,
			Unknown=256
		};

		enum Level {
			Level_Fast=0,
			Level_Normal,
			Level_Default,
			Level_High
		};

		enum Prefix {
			Prefix_None=0,
			Prefix_V1,
			Prefix_V2,
		};
	private:
		void *buffer;
		void *uncbuffer;
		Algorithm aaa;
		Level lll;
		Prefix prefix;

		void doNone(void *dst, size_t *dstlen, const void *src, size_t size);
		void doZlib(void *dst, size_t *dstlen, const void *src, size_t size);
		void doBzip2(void *dst, size_t *dstlen, const void *src, size_t size);

		void unNone (void *dst, size_t *dstlen, const void *src, size_t srclen);
		void unZlib (void *dst, size_t *dstlen, const void *src, size_t srclen);
		void unBzip2 (void *dst, size_t *dstlen, const void *src, size_t srclen);


	public:

		Compression();
		Compression(Algorithm method, Level level=Level_Default);
		~Compression();
		void init(Algorithm method, Level level=Level_Default);
		void usePrefix(Prefix prefix);

		void compress(void *dst, size_t *dstlen, const void *src, size_t size, Algorithm a=Unknown);
		void compress(ByteArray &out, const void *ptr, size_t size);
		void compress(ByteArray &out, const ByteArrayPtr &in);
		ByteArrayPtr compress(const void *ptr, size_t size);
		ByteArrayPtr compress(const ByteArrayPtr &in);

		void uncompress(void *dst, size_t *dstlen, const void *src, size_t srclen, Algorithm a=Unknown);
		void uncompress(ByteArray &out, const ByteArrayPtr &data);
		void uncompress(ByteArray &out, const void *data, size_t size=0);
		ByteArrayPtr uncompress(const void *ptr, size_t size);
		ByteArrayPtr uncompress(const ByteArrayPtr &in);
};

void Compress(ByteArray &out, const ByteArrayPtr &in, Compression::Algorithm method, Compression::Level level=Compression::Level_Default);
void CompressZlib(ByteArray &out, const ByteArrayPtr &in, Compression::Level level=Compression::Level_Default);
void CompressBZip2(ByteArray &out, const ByteArrayPtr &in, Compression::Level level=Compression::Level_Default);
void Uncompress(ByteArray &out, const ByteArrayPtr &in);



class PFPChunk
{
	friend class PFPFile;
	private:
		String chunkname;
		void *chunkdata;
		size_t chunksize;
	public:
		PFPChunk();
		~PFPChunk();
		void setName(const String &chunkname);
		void setData(const void *ptr, size_t size);
		void setData(const ByteArrayPtr &data);
		void setData(const String &s);
		void setData(const char *s);
		size_t size();
		const void *data();
		const String &name();
};

class PFPFile
{
	private:
		List<PFPChunk*> Chunks;
		String id;
		ppluint8 mainversion, subversion;
		Compression::Algorithm comp;


		void setParam(const String &chunkname, const String &data);
		void saveChunk(char *buffer, size_t &pp, PFPChunk *chunk);


	public:
		class Iterator : public  List<PFPChunk*>::Iterator
		{
			public:
			String findchunk;
		};

		Mutex	myMutex;

		PFPFile();
		virtual ~PFPFile();
		void clear();
		void setAuthor(const String &author);
		void setCopyright(const String &copy);
		void setDescription(const String &descr);
		void setName(const String &name);
		void setVersion(int main=0, int sub=0);
		void setId(const String &id);
		void save(const String &filename);
		void addChunk(PFPChunk *chunk);
		void deleteChunk(PFPChunk *chunk);
		void deleteChunk(const String &chunkname);
		PFPChunk *findFirstChunk(Iterator &it, const String &chunkname) const;
		PFPChunk *findNextChunk(Iterator &it, const String &chunkname) const;
		virtual void list() const;
		void setCompression(Compression::Algorithm type);

		void load(FileObject &ff);
		void load(const String &file);

		bool ident(FileObject &ff);
		bool ident(const String &file);

		virtual int loadRequest(const String &id, int mainversion ,int subversion);

		String getName() const;
		String getDescription() const;
		String getAuthor() const;
		String getCopyright() const;
		void getVersion(int *main, int *sub) const;
		const String &getID() const;
		int getMainVersion() const;
		int getSubVersion() const;
		Compression::Algorithm getCompression() const;

		void reset(Iterator &it) const;
		PFPChunk *getFirst(Iterator &it) const;
		PFPChunk *getNext(Iterator &it) const;

};

//! \brief PPL-Resourcen
class Resource
{
	private:
		int count;
		int maxid, minid;
		int major, minor;
		ByteArray memory;
		ByteArrayPtr memref;
		void *firstchunk;

		void checkResource(const ByteArrayPtr &memory);
		void parse();
		void uncompress(void *resource);

		void *find(int id);
		void *find(const String &name);

	public:
		Resource();
		~Resource();
		void clear();
		void list();
		void load(const String &filename);
		void load(FileObject &file);
		void load(const ByteArrayPtr &memory);
			/* Bei Load wird die gesamte Datei in den Speicher geladen und erst
			 * beim Loeschen der Resource wieder freigegeben. Die angegebene Datei
			 * kann vorher geschlossen werden
			 */
		void useMemory(const ByteArrayPtr &memory);
		void useMemory(void *ptr, size_t size);
			/* Wird UseMemory benutzt, muss ein Pointer auf einen Speicherbereich
			 * angegeben werden, der seine Gueltigkeit nicht verlieren darf,
			 * solange die Instanz von Resource existiert.
			 */

		FileObject *getFile(int id);
		FileObject *getFile(const String &name);
		ByteArrayPtr getMemory(int id);
		ByteArrayPtr getMemory(const String &name);

		static Resource *getPPLResource();
		static void generateResourceHeader(const String &basispfad, const String &ConfigParser, const String &targetfile, const String &label);
};

Resource *GetPPLResource();


class PerlHelper
{
	public:
		static String escapeString(const String &s);
		static String escapeRegExp(const String &s);
		static String toHash(const AssocArray &a, const String &s);

};


#define NUMFACILITIES	9
#ifdef ERROR
#undef ERROR
#endif
#ifdef DEBUG
#undef DEBUG
#endif

class LogHandler;

//! \brief Allgemeine Logging-Klasse
class Logger
{
	public:
		enum PRIORITY {
			EMERG		= 1,
			ALERT		= 2,
			CRIT		= 3,
			ERR			= 4,
			ERROR		= 4,
			WARNING		= 5,
			NOTICE		= 6,
			INFO		= 7,
			DEBUG		= 8
		};

		enum SYSLOG_FACILITY {
			SYSLOG_AUTH=1,
			SYSLOG_AUTHPRIV,
			SYSLOG_CONSOLE,
			SYSLOG_CRON,
			SYSLOG_DAEMON,
			SYSLOG_FTP,
			SYSLOG_KERN,
			SYSLOG_LPR,
			SYSLOG_MAIL,
			SYSLOG_NEWS,
			SYSLOG_NTP,
			SYSLOG_SECURITY,
			SYSLOG_SYSLOG,
			SYSLOG_USER,
			SYSLOG_UUCP,
			SYSLOG_LOCAL0,
			SYSLOG_LOCAL1,
			SYSLOG_LOCAL2,
			SYSLOG_LOCAL3,
			SYSLOG_LOCAL4,
			SYSLOG_LOCAL5,
			SYSLOG_LOCAL6,
			SYSLOG_LOCAL7
		};

	private:
		Mutex		mutex;
		AssocArray	*FilterModule, *FilterFile;
		String		ProgIdentity;
		int			debuglevel[NUMFACILITIES];
		bool		console_enabled;
		PRIORITY	console_priority;
		int			console_level;
		File		logff[NUMFACILITIES];
		String		logfilename[NUMFACILITIES];
		void		*firsthandler, *lasthandler;
		bool		logconsole;
		bool		logThreadId;
		int			rotate_mechanism;
		ppluint64	maxsize;
		int			generations;
		bool		inrotate;
		bool			useSyslog;
		SYSLOG_FACILITY	syslogFacility;
		String			syslogIdent;

		bool		shouldPrint(const char *module, const char *function, const char *file, int line, PRIORITY prio, int level);
		int			isFiltered(const char *module, const char *function, const char *file, int line, int level);
		void		output(PRIORITY prio, int level, const char *module, const char *function, const char *file, int line, const String &buffer, bool printdate=true);
		void		outputArray(PRIORITY prio, int level, const char *module, const char *function, const char *file, int line, const AssocArray &a, const char *prefix, String *Out=NULL);
		void		checkRotate(PRIORITY prio);

	public:

		Logger();
		~Logger();
		void	terminate();
		void	addLogHandler(LogHandler *handler);
		void	deleteLogHandler(LogHandler *handler);
		void	setLogfile(PRIORITY prio, const String &filename);
		void	setLogfile(PRIORITY prio, const String &filename, int level);
		void	setLogLevel(PRIORITY prio, int level=1);
		int		getLogLevel(PRIORITY prio);
		String	getLogfile(PRIORITY prio);
		void	setLogRotate(ppluint64 maxsize, int generations);
		void	enableConsole(bool flag=true, PRIORITY prio=Logger::DEBUG, int level=1);
		void	openSyslog(const String &ident, SYSLOG_FACILITY facility=SYSLOG_USER);
		void	closeSyslog();
		void	printException(const Exception &e);
		void	printException(const char *file, int line, const Exception &e);
		void	printException(const char *file, int line, const char *module, const char *function, const Exception &e);
		void	print (const String &text);
		void	print (int level, const String &text);
		void	print (PRIORITY prio, int level, const String &text);
		void	print (PRIORITY prio, int level, const char *file, int line, const String &text);
		void	print (PRIORITY prio, int level, const char *module, const char *function, const char *file, int line, const String &text);
		void	printArray (PRIORITY prio, int level, const AssocArray &a, const String &text);
		void	printArray (PRIORITY prio, int level, const char *module, const char *function, const char *file, int line, const AssocArray &a, const String &text);
		void	printArraySingleLine (PRIORITY prio, int level, const char *module, const char *function, const char *file, int line, const AssocArray &a, const String &text);
		void	hexDump (PRIORITY prio, int level, const void * address, int bytes);
		void	hexDump (const void * address, int bytes);
		void	setFilter(const char *module, const char *function, int level);
		void	setFilter(const char *file, int line, int level);
		void	deleteFilter(const char *module, const char *function);
		void 	deleteFilter(const char *file, int line);
};

//! \brief Log-Handler
class LogHandler
{
	public:
		virtual ~LogHandler()=0;
		virtual void logMessage(Logger::PRIORITY prio, int level, const String &msg)=0;
};



class ConfigParser
{
	private:
		String  separator;
		String  currentsection;
		void	*first, *last, *section;
		AssocArray sections;
		AssocArray::Iterator it;

		void *findSection(const String &sectionname) const;

	public:
		ConfigParser();
		ConfigParser(const String &filename);
		ConfigParser(FileObject &file);
		~ConfigParser();
		void load(const String &filename);
		void load (FileObject &file);
		void loadFromString(const String &string);
		void loadFromMemory(const void *buffer, size_t bytes);
		void loadFromMemory(const ByteArrayPtr &ptr);
		void save(const String &filename);
		void save(FileObject &file);
		void unload();
		void setSeparator(const String &string);
		const String &getSeparator() const;
		void selectSection(const String &section);
		int  firstSection();
		int  nextSection();
		const String& getSectionName() const;
		const String& getSection(const String &name) const;
		void copySection(AssocArray &target, const String &section) const;
		void createSection (const String &name);
		void deleteSection (const String &name);
		void add(const String &section, const String &key, const String &value);
		void add(const String &section, const String &key, const char *value);
		void add(const String &key, const String &value);
		void add(const String &key, const char *value);
		void add(const String &key, int value);
		void add(const String &key, bool value);
		void add(const String &section, const String &key, int value);
		void add(const String &section, const String &key, bool value);
		void deleteKey(const String &key);
		void deleteKey(const String &section, const String &key);
		String get(const String &key, const String &defaultvalue=String());
		bool	getBool(const String &key, bool defaultvalue=false);
		int		getInt(const String &key, int defaultvalue=0);
		String	getFromSection(const String &section, const String &key, const String &defaultvalue=String());
		bool	getBoolFromSection(const String &section, const String &key, bool defaultvalue=false);
		int		getIntFromSection(const String &section, const String &key, int defaultvalue=0);
		void reset();								// Zum Auslesen einer kompletten Section
		bool getFirst(String &key, String &value);
		bool getNext(String &key, String &value);
		void print() const;
};


};	// EOF namespace ppl7


 #endif	// #ifndef _PPL7_INCLUDE
