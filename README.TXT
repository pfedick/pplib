PPLib - Patrick's Programming Library
===========================================================================

INDEX

1. What is PPLib?
2. Installation
2.1 UNIX
2.2 Windows
3. Usage
4. Documentation
5. Credits
6. Copyright
7. Licence

===========================================================================
1. What is PPLib?

   PPLib (or PPL) stands for "Patrick's Programming Library" and is a
   collection of functions and classes for C++ applications, written by
   Patrick Fedick. It covers file and directory functions, strings and
   arrays, time, math, threads, graphics, sound, database access and
   internet communication.

   You can find the newest version and documentation of this library under

       http://www.pfp.de/pplib


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
    which PPLib can make use of (e.g. graphic libraries or databases).

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
    PPLib comes with project files for Microsoft Visual Studio .NET 2003,
    but you have to do some manual configuration to adapt it to your
    system.

   For more details and examples about installation, please read the HTML
   documentation. 
   
===========================================================================
3. Usage

   How to use the functions and classes if PPLib is described in the
   HTML documentation.

===========================================================================
4. Documentation

   Documentation is available in HTML and can be found in the subdirectory
   "documentation/html" of the source distribution and and online on the
   PPLib webpage:
   
        http://www.pfp.de/pplib

===========================================================================
5. Credits

   Some parts of PPLib includes code from other projects or can make use of
   third party libraries.
   - MD5-Hash calculation
     Copyright RSA Data Security, Inc. MD5 Message-Digest Algorithm)
   - random number generation
     Copyright (c) 1983, 1993 The Regents of the University of California
   - CRC32 checksum calculation
     unknown source
   - GIF-Reader
     Copyright 1990, 1991, 1993 by David Koblas
   - PCRE2 (http://www.pcre.org/)
     Copyright (c) 1997-2008 University of Cambridge All rights reserved.
   - Zlib (http://www.zlib.net/)
     Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler
   - Bzip2 (http://www.bzip.org/)\n
     Copyright (C) 1996-2007 Julian R Seward

===========================================================================
6. Copyright

   PPLib is copyright by Patrick Fedick <patrick@pfp.de> in 2005-2019
   All rights reserved.

===========================================================================
7. Licence
 

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer. 
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



