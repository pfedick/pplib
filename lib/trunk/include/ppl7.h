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


#ifndef _PPL7_INCLUDE
#define _PPL7_INCLUDE

#define PPL_VERSION_MAJOR	7
#define PPL_VERSION_MINOR	0
#define PPL_VERSION_BUILD	0
#define PPL_RELEASEDATE		20110421

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

 class Heap
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



 		Heap();
 		Heap(size_t elementsize, size_t startnum, size_t increase, size_t growpercent=30);
 		~Heap();
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

String StripSlashes(const String &str);
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
		HAVE_MMX2		= 0x00000004,
		HAVE_3DNow		= 0x00000008,
		HAVE_3DNow2		= 0x00000010,
		HAVE_SSE		= 0x00000020,
		HAVE_SSE2		= 0x00000040,
		HAVE_AMD64		= 0x00000080,
		HAVE_SSE3		= 0x00000100,
		HAVE_SSSE3		= 0x00000200,
		HAVE_SSE4a		= 0x00000400,
		HAVE_SSE41		= 0x00000800,
		HAVE_SSE42		= 0x00001000
	};
}

typedef struct {
	ppluint32	caps;							// Struktur kann um weitere Informationen erweitert werden
	ppluint32	bits;

} CPUCaps;

ppluint32 GetCPUCaps (CPUCaps *cpu);				// Wenn cpu=NULL ist, werden nur die Caps zur�ckgegeben



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
		ppluint64	threadId;
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

		virtual void threadMain();
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
		PPLNORMALEXCEPTION(UnimplementedVirtualFunctionException);
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
		void				putws (const String &str);
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
		char * buffer;
		size_t	mysize;
		size_t	pos;
		char * MemBase;

	public:

		MemFile ();
		MemFile (void * adresse, size_t size);
		MemFile (const ByteArrayPtr &memory);
		~MemFile();

		void		open(void * adresse, size_t size);
		void		open(const ByteArrayPtr &memory);
		char		*adr(size_t adresse);

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
		//virtual int		GetFileNo() const;
		virtual void		flush();
		virtual void		sync();
		//virtual int		Truncate(ppluint64 length);
		virtual bool		isOpen() const;
		//virtual int		LockShared(bool block=true);
		//virtual int		LockExclusive(bool block=true);
		//virtual int		Unlock();
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

		static void remove(const String &filename);
		static void remove(const char *filename);
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

		typedef ppl7::List<const DirEntry*>::Iterator Iterator;

		Dir();
		Dir(const char *path, Sort s=SORT_NONE);
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
		void print() const;
		void print(const DirEntry &de) const;
		static String currentPath();
		//static CString HomePath();
		//static CString TempPath();

};




 };	// EOF namespace ppl7


 #endif	// #ifndef _PPL7_INCLUDE
