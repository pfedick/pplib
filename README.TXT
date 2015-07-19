PPL6 - README
===========================================================================

INDEX

1. What is PPL6?
2. Installation
2.1 UNIX
2.2 Windows
3. Usage
3.1 Unix
3.2 Windows
4. Documentation
5. Credits
6. Copyright
7. Licence

===========================================================================
1. What is PPL6?

   PPL stands for "Patrick's Programming Library" and is a collection of
   usefull functions and classes for C++ applications, written by Patrick
   Fedick. It covers file and directory functions, strings and arrays,
   time, math, threads, graphics and sound.

   You can find the newest version and documentation of this library under

       http://www.pfp.de/ppl


===========================================================================
2. Installation

2.1 UNIX
    The UNIX version comes with a standard configure-script. In most
    cases it is sufficient to simply do

        ./configure
        make
        make install    (do this as root)

    The configure script tries to find out on what system the library
    should be compiled and what optional libraries are available of
    which ppl can make use of (e.g. graphic libraries or databases).

    If it does not find a library of which you know that it is
    installed somewhere, you can manually set a search path for it
    during configure, e.g.:

        ./configure --with-mysql=/usr/local/mysql

    If you don't want a specific library compiled in, you can deselect
    it with "--without-xxx', where xxx stands for the library. Example:

        ./configure --without-mysql

    Please see "./configure --help" for a list of all available
    options.

2.2 Windows
    PPL comes with project files for Microsoft Visual Studio .NET 2003,
    but you have to do some manual configuration to adapt it to your
    system.


===========================================================================
3. Usage

   To make use of the functions in ppl6 you have to do two things:
      1. include the required header files in your program, e.g.
         #include <ppl6.h>
         #include <ppl6-grafix.h>
      2. include the required compiler options in your makefile,
         which is different on unix and windows platforms (see below).

   After including the header files, you can access the functions either
   by typing "ppl6::FUNCTIONNAME" everytime you need something from the
   lib, or by writing "using namespace ppl6;" somewhere in your header
   or program files. Please keep in mind, that in this case all functions
   from ppl6 are visible in the global namespace and it is possible that
   you get conflicting function names.
 
3.1 Unix
3.2 Windows

===========================================================================
4. Documentation

   Documentation is available in HTML and PDF on the PPL webpage:
   
        http://www.pfp.de/ppl

===========================================================================
5. Credits

   Some parts of PPL includes code from other projects or can make use of
   third party libraries.
   - MD5-Hash calculation
     Copyright RSA Data Security, Inc. MD5 Message-Digest Algorithm)
   - random number generation
     Copyright (c) 1983, 1993 The Regents of the University of California
   - CRC32 checksum calculation
     unknown source
   - GIF-Reader
     Copyright 1990, 1991, 1993 by David Koblas
   - PCRE (http://www.pcre.org/)
     Copyright (c) 1997-2008 University of Cambridge All rights reserved.
   - Zlib (http://www.zlib.net/)
     Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler
   - Bzip2 (http://www.bzip.org/)\n
     Copyright (C) 1996-2007 Julian R Seward

===========================================================================
6. Copyright

   PPL6 is copyright by Patrick Fedick <patrick@pfp.de> in 2005-2008
   All rights reserved.

===========================================================================
7. Licence
 
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.


