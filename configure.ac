###############################################################################
# This file is part of "Patrick's Programming Library", Version 7 (PPL7).
# Web: https://github.com/pfedick/pplib
###############################################################################
# Copyright (c) 2024, Patrick Fedick <patrick@pfp.de>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#    1. Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.
###############################################################################

dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.71])
AC_INIT([ppl],[7.1.0],[patrick@pfp.de],[ppl-7.1.0.tar.bz2])
AC_CONFIG_AUX_DIR(autoconf)

AC_CONFIG_SRCDIR([include/ppl7.h])
AC_CONFIG_SRCDIR([include/config_ppl7.h.in])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([tests/Makefile])
AC_CONFIG_FILES([ppl7-config])
AC_CONFIG_HEADERS([include/config_ppl7.h])
AC_CONFIG_HEADERS([include/ppl7-config.h])


###############################################################################
# Includes
###############################################################################
sinclude(autoconf/ax_cxx_compile_stdcxx.m4)
sinclude(autoconf/ax_check_openssl.m4)
sinclude(autoconf/libcurl.m4)
sinclude(autoconf/ax_lib_mysql.m4)
sinclude(autoconf/ax_lib_postgresql.m4)
sinclude(autoconf/ax_lib_sqlite3.m4)
sinclude(autoconf/freetype.m4)
sinclude(autoconf/ax_path_lib_pcre.m4)
sinclude(autoconf/png.m4)
sinclude(autoconf/jpeg.m4)
sinclude(autoconf/libjpeg-turbo.m4)
sinclude(autoconf/ax_pthread.m4)
sinclude(autoconf/nasm.m4)
sinclude(autoconf/lib-link.m4)
sinclude(autoconf/iconv.m4)
sinclude(autoconf/bzip2.m4)
sinclude(autoconf/ax_check_zlib.m4)
sinclude(autoconf/ax_lib_crypto.m4)
sinclude(autoconf/checkfuncs.m4)
sinclude(autoconf/ax_gcc_x86_cpuid.m4)
sinclude(autoconf/ax_check_compile_flag.m4)
sinclude(autoconf/ax_check_compiler_flags.m4)
sinclude(autoconf/ax_gcc_archflag.m4)
sinclude(autoconf/ldns.m4)
sinclude(autoconf/mpg123.m4)
sinclude(autoconf/lame.m4)
sinclude(autoconf/vorbis.m4)
#sinclude(autoconf/opus.m4)
sinclude(autoconf/shout.m4)
sinclude(autoconf/imagemagick.m4)
sinclude(autoconf/libtiff.m4)
sinclude(autoconf/libmicrohttpd.m4)
sinclude(autoconf/libidn.m4)
sinclude(autoconf/libidn2.m4)
sinclude(autoconf/libbind.m4)
sinclude(autoconf/gtest.m4)
sinclude(autoconf/libcdio.m4)



report_have_pthreads="no"
report_have_jpeg="no"
report_have_libmicrohttpd="no"
report_have_libcrypto="no"
report_have_libidn="no"
report_have_libidn2="no"
report_have_libtiff="no"
report_have_curl="no"
report_have_mpg123="no"
report_have_lame="no"
report_have_libvorbis="no"
report_have_libshout="no"
report_have_libmad="no"
report_have_iconv="no"
report_have_bzip2="no"
report_have_zlib="no"
report_have_pcre="no"
report_have_openssl="no"
report_have_freetype2="no"
report_have_png="no"
report_have_libimlib="no"
report_have_mysql="no"
report_have_postgres="no"
report_have_sqlite3="no"
report_have_ldns="no"
report_have_nasm="no"


AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE

OLD_CFLAGS=$CFLAGS
# Checks for programs.
AC_PROG_CC
CFLAGS=$OLD_CFLAGS
AC_PROG_CXX

AX_CXX_COMPILE_STDCXX_17


AX_CHECK_COMPILE_FLAG([-no-pie],
    LDFLAGS="-no-pie $LDFLAGS"
	CFLAGS="$CFLAGS -fPIE "
    NOPIE="-no-pie",
    NOPIE=""
)
AC_SUBST(NOPIE)



case "$host_os" in
	freebsd* )
		CFLAGS="-I/usr/local/include $CFLAGS"
		LDFLAGS="-L/usr/local/lib $LDFLAGS"
  		;;
esac

case "$host_os" in
	*mingw32*|msys )
		report_have_pthreads="yes, Windows Threads"
  		;;
  	*mingw64* )
		report_have_pthreads="yes, Windows Threads"
  		;;
	*)
		AX_PTHREAD([report_have_pthreads="yes, pthreads"]
			AC_DEFINE(HAVE_PTHREADS,1, [Define to 1 if we have pthreads])
		)
		;;
esac

AC_PROG_SED

AC_C_BIGENDIAN(
	report_byteorder="BIG ENDIAN"
	AC_DEFINE(__BIG_ENDIAN__),
	report_byteorder="LITTLE ENDIAN"
	AC_DEFINE(__LITTLE_ENDIAN__))

AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(wchar_t)
AC_CHECK_SIZEOF(size_t)
AC_CHECK_SIZEOF(float)
AC_CHECK_SIZEOF(double)
AC_CHECK_SIZEOF(void*)

AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINTPTR_T


if test $ac_cv_sizeof_wchar_t -eq 4; then
	if test "$report_byteorder" = "LITTLE ENDIAN"; then
		AC_DEFINE(ICONV_UNICODE,"UTF-32LE")
	else
		AC_DEFINE(ICONV_UNICODE,"UTF-32BE")
	fi
else
	if test "$report_byteorder" = "LITTLE ENDIAN"; then
		AC_DEFINE(ICONV_UNICODE,"UTF-16LE")
	else
		AC_DEFINE(ICONV_UNICODE,"UTF-16BE")
	fi
fi



AC_CHECK_SIZEOF(fpos_t)


AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

# AX_HAVE_QT scheint unter FreeBSD nicht kompatibel zu sein. Der produzierte Code
# läuft zwar unter FreeBSD, aber nicht unter Linux oder MinGW
dnl AX_HAVE_QT

PKG_PROG_PKG_CONFIG()


#--------------------------------------------------------------------
#       Check for various typedefs and provide substitutes if
#       they don't exist.
#--------------------------------------------------------------------

AC_HEADER_STDBOOL
AC_CHECK_HEADERS_ONCE([sys/time.h])
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_TYPE_OFF_T
dnl AC_CHECK_SIZEOF([off_t])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM
# FPOS_T ist ein Struct?
AX_FPOS_ISSTRUCT()

AX_TM_HAS_GMTOFF()

AC_PATH_XTRA


AC_ARG_VAR(GCOV_CFLAGS, [additional compiler flags to compile the code coverage version of the library] )
AC_ARG_VAR(GCOV_LDLAGS, [additional linker flags to compile the code coverage version of the library] )
AC_SUBST(GCOV_CFLAGS)
AC_SUBST(GCOV_LDFLAGS)
PPLIBTESTVERSION=release
report_codecoverage_enabled="no"

AC_ARG_ENABLE([coverage], [  --enable-coverage       Enable build of code coverage version and tests],
AS_IF([test "x$enable_coverage" != "xno"], [
	PPLIBTESTVERSION=coverage
	AC_SUBST(PPLIBTESTVERSION)
	report_codecoverage_enabled="yes"
],[
PPLIBTESTVERSION=debug
AC_SUBST(PPLIBTESTVERSION)
]),
[
PPLIBTESTVERSION=debug
AC_SUBST(PPLIBTESTVERSION)
]
)



report_have_gtest="no"
AC_CHECK_GTEST(
	report_have_gtest="yes"
	,
	report_have_gtest="no"
	report_codecoverage_enabled="no"
)


AC_CHECK_HEADERS([stdlib.h stdint.h inttypes.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([stdarg.h])
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([stdio.h])
AC_CHECK_HEADERS([math.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([sys/file.h])
AC_CHECK_HEADERS([time.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/poll.h])
AC_CHECK_HEADERS([sys/select.h])
AC_CHECK_HEADERS([sys/sysinfo.h])
AC_CHECK_HEADERS([sys/sysctl.h])
AC_CHECK_HEADERS([sys/resource.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([arpa/nameser.h])
AC_CHECK_HEADERS([resolv.h],[],[],
[
#ifdef HAVE_SYS_TYPES_H
	#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
	#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
	#include <arpa/nameser.h>
#endif
])


AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([strings.h])
AC_CHECK_HEADERS([errno.h])
AC_CHECK_HEADERS([signal.h])
AC_CHECK_HEADERS([dirent.h])
AC_CHECK_HEADERS([fnmatch.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([limits.h])
AC_CHECK_HEADERS([ctype.h])
AC_CHECK_HEADERS([wctype.h])
AC_CHECK_HEADERS([locale.h])
AC_CHECK_HEADERS([wchar.h])
AC_CHECK_HEADERS([widec.h])
AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([pthread.h pthread_np.h sched.h])
AC_CHECK_HEADERS([sys/mman.h])
AC_CHECK_HEADERS([valgrind/helgrind.h])

AC_FUNC_MKTIME
AC_CHECK_TYPE(size_t, unsigned)
AC_CHECK_TYPE(wchar_t, unsigned)

# Checks for library functions.
AC_FUNC_FSEEKO
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([strtolower strtoupper])
AC_CHECK_FUNCS([bzero bcopy gethostbyname gettimeofday inet_ntoa memmove memset mkdir socket __mingw_vasprintf vasprintf asprintf])
AC_CHECK_FUNCS([gethostname sethostname getdomainname setdomainname uname])
AC_CHECK_FUNCS([strcasecmp strlen strnlen strdup strerror strncasecmp strndup strstr strcasestr strcmp strncmp])
AC_CHECK_FUNCS([strncpy strlcpy strncat strlcat])
AC_CHECK_FUNCS([strtok strtok_r])
AC_CHECK_FUNCS([htol atoll atoi atol atof strtoll strtoull])
AC_CHECK_FUNCS([flock fcntl truncate ftruncate sync fsync unlink remove])
AC_CHECK_FUNCS([fgets fputs fgetws fputws fputc fgetc fputwc fgetwc mmap munmap sysconf pagesize])
AC_CHECK_FUNCS([usleep nanosleep localtime localtime_r gmtime gmtime_r])
AC_CHECK_FUNCS([signal syslog openlog closelog])
AC_CHECK_FUNCS([strtok_r])
AC_CHECK_FUNCS([getpid getppid getuid geteuid])
AC_CHECK_FUNCS([mkstemp])
AC_CHECK_FUNCS([wcscmp wcsncmp wcscasecmp wcsncasecmp wcstol wcstoll wcslen wcsstr wcstoul wcstoull wcstod wcstof])
AC_CHECK_FUNCS([wcstombs mbstowcs wcsrtombs wcsnrtombs mbsrtowcs mbsnrtowcs])
AC_CHECK_FUNCS([wprintf fwprintf swprintf vwprintf vfwprintf vswprintf])
AC_CHECK_FUNCS([vsnprintf])
AC_CHECK_FUNCS([wstol wstoll watoi watoll wstod watof])
AC_CHECK_FUNCS([poll select pselect])
AC_CHECK_FUNCS([opendir closedir readdir readdir_r stat])
AC_CHECK_FUNCS([clock_gettime])
AC_CHECK_FUNCS([inet_ntop inet_pton inet_aton inet_ntoa inet_ntoa_r inet_addr inet_network inet_makeaddr])

AC_CHECK_LIB(pthread, pthread_set_name_np,
	AC_DEFINE(HAVE_PTHREAD_SET_NAME_NP,1),,)
AC_CHECK_LIB(pthread, pthread_setname_np,
	AC_DEFINE(HAVE_PTHREAD_SETNAME_NP,1),,)

AC_CHECK_FUNCS([sched_yield pthread_yield])



AC_CACHE_CHECK([for sa_len in struct sockaddr], ac_cv_struct_sa_len,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <sys/socket.h>]], [[struct sockaddr s; s.sa_len;]])],[ac_cv_struct_sa_len=yes],[ac_cv_struct_sa_len=no])])
if test $ac_cv_struct_sa_len = yes; then
  AC_DEFINE(HAVE_SOCKADDR_SA_LEN)
fi


# Resolver
AM_CHECK_LIBBIND()
report_have_libbind="no"
if test "$have_res_query" = "yes"
then
	AC_DEFINE(HAVE_LIBBIND,1,[ Define if you have libbind resolver functions ])
	report_have_libbind="yes"
fi


LDFLAGS="$LDFLAGS -L/usr/local/lib"
CFLAGS="$CFLAGS -I/usr/local/include"


##################################################################################### JPEG
jpeg_note=""
AC_CHECK_LIBJPEG([6.0],
            AC_DEFINE(HAVE_JPEG, 1, [ Define if you have libjpeg. ])
            report_have_jpeg="yes"
           :)


##################################################################################### JPEG-TURBO
AC_CHECK_LIBJPEGTURBO([1.3.0],
            AC_DEFINE(HAVE_LIBJPEGTURBO, 1, [ Define if you have libjpeg-turbo. ])
            AC_DEFINE(HAVE_JPEG,1)
            report_have_jpeg="yes"
            jpeg_note="(using libjpeg-turbo)"
			JPEG_LIBS="$JPEG_LIBS $TURBOJPEG_LIBS"
			JPEG_CFLAGS="$JPEG_CFLAGS $TURBOJPEG_CFLAGS"

           :)
AC_SUBST(JPEG_CFLAGS)
AC_SUBST(JPEG_LIBS)


##################################################################################### PNG
AC_CHECK_LIBPNG([1.2],
            AC_DEFINE(HAVE_PNG, 1, [ Define if you have libpng. ])
            report_have_png="yes"
           :)


AM_LIBMICROHTTPD(,
            AC_DEFINE(HAVE_LIBMICROHTTPD, 1, [ Define if you have libmicrohttpd. ])
            report_have_libmicrohttpd="yes"
            AC_SUBST(LIBMICROHTTPD_CFLAGS)
			AC_SUBST(LIBMICROHTTPD_LIBS)

            ,
            :)

LIBIDN_CFLAGS=
LIBIDN_LIBS=
AM_LIBIDN([0.5.4],
	report_have_libidn="yes"
	AC_DEFINE(HAVE_LIBIDN, 1, [ Define if you have libidn. ])
	,
	)



AM_LIBIDN2([2.0.0],
	report_have_libidn2="yes"
	AC_DEFINE(HAVE_LIBIDN2, 1, [ Define if you have libidn2. ])
	LIBIDN_CFLAGS="$LIBIDN2_CFLAGS"
	LIBIDN_LIBS="$LIBIDN2_LIBS"
	,
	report_have_libidn2="no"
	)
AC_SUBST(LIBIDN_CFLAGS)
AC_SUBST(LIBIDN_LIBS)



AC_CHECK_LIBTIFF([1.2],
            AC_DEFINE(HAVE_TIFF, 1, [ Define if you have libtiff. ])
            report_have_libtiff="yes"
            AC_SUBST(TIFF_CFLAGS)
			AC_SUBST(TIFF_LIBS)
           :)


report_have_curl="no"
LIBCURL_CHECK_CONFIG([yes],[7.30.0],
	#LIBS="$LIBS $LIBCURL"
	#CFLAGS="$CFLAGS $LIBCURL_CPPFLAGS "
	AC_SUBST(LIBCURL)
	AC_SUBST(LIBCURL_CPPFLAGS)
	report_have_curl="yes",
	:)



AC_CHECK_LIBMPG123([1.0.0],
            AC_DEFINE(HAVE_MPG123, 1, [ Define if you have mpg123 library. ])
            report_have_mpg123="yes"
			AC_SUBST(MPG123_CFLAGS)
			AC_SUBST(MPG123_LIBS)
            ,
            :)


AC_CHECK_LIBMP3LAME([3.98.0],
	AC_DEFINE(HAVE_LAME, 1, [ Define if you have lame. ])
	report_have_lame="yes"
	AC_SUBST(LAME_CFLAGS)
	AC_SUBST(LAME_LIBS)
	, :)

XIPH_PATH_VORBIS(
	AC_DEFINE(HAVE_LIBVORBIS, 1, [ Define if you have libvorbis. ])
	report_have_libvorbis="yes"
	, :)

XIPH_PATH_SHOUT(
	AC_DEFINE(HAVE_LIBSHOUT, 1, [ Define if you have libshout. ])
	report_have_libshout="yes"
	AC_SUBST(SHOUT_CFLAGS)
	AC_SUBST(SHOUT_LIBS)
	, :)

report_have_libcdio="no"
AX_CHECK_LIBCDIO(
        report_have_libcdio="yes",
        :)



AM_ICONV
if test "$am_cv_func_iconv_works" = "yes"
then
	report_have_iconv="yes"
	ICONV_CFLAGS=$INCICONV
	ICONV_LIBS=$LIBICONV
	AC_SUBST(ICONV_CFLAGS)
	AC_SUBST(ICONV_LIBS)
fi


# Bzip2 Vorhanden?
AM_BZIP2
AX_CHECK_ZLIB(report_have_zlib="yes"
	AC_DEFINE([HAVE_LIBZ], [1], [Define to 1 if you have `z' library (-lz)])
	ZLIB_CFLAGS="-I${ZLIB_HOME}/include"
	ZLIB_LIBS="-L${ZLIB_HOME}/lib -lz"
	AC_SUBST(ZLIB_CFLAGS)
	AC_SUBST(ZLIB_LIBS)
)


dnl do we need to specify -lm explicitly?
AC_CHECK_FUNC(sin,,[AC_CHECK_LIB(m,sin)])

# PCRE vorhanden?
AX_PATH_LIB_PCRE(
            AC_DEFINE(HAVE_PCRE, 1, [ Define if you have pcre. ])
            report_have_pcre="yes"
            ,
            )

AX_CHECK_OPENSSL(
	AC_DEFINE(HAVE_OPENSSL, 1, [ Define if you have openssl. ])
	report_have_openssl="yes"

	am_save_CPPFLAGS="$CPPFLAGS"
	am_save_CFLAGS="$CFLAGS"
	am_save_LIBS="$LIBS"
	am_save_LDFLAGS="$LDFLAGS"

	LIBS="$LIBS $OPENSSL_LIBS"
	CFLAGS="$CFLAGS $OPENSSL_INCLUDES"

	AC_CHECK_FUNCS([TLS_method TLS_server_method TLS_client_method])
	AC_CHECK_FUNCS([EVP_aes_128_ecb EVP_aes_192_ecb EVP_aes_256_ecb EVP_aria_128_ecb EVP_aria_192_ecb EVP_aria_256_ecb])
	AC_CHECK_FUNCS([EVP_bf_ecb EVP_camellia_128_ecb EVP_camellia_192_ecb EVP_camellia_256_ecb EVP_cast5_ecb])
	AC_CHECK_FUNCS([EVP_des_ecb EVP_des_ede3_ecb EVP_idea_ecb EVP_rc2_ecb EVP_rc5_32_12_16_ecb])
	AC_CHECK_FUNCS([SSLv23_method SSLv23_server_method SSLv23_client_method])


	CPPFLAGS="$am_save_CPPFLAGS"
	CFLAGS="$am_save_CFLAGS"
	LIBS="$am_save_LIBS"
	LDFLAGS="$am_save_LDFLAGS"

	, :)


AC_CHECK_FT2(9.0.0,
	AC_DEFINE(HAVE_FREETYPE2, 1, [ Define if you have freetype2. ])
	report_have_freetype2="yes"
	, :)


AX_PATH_LIB_IMLIB([1.9.0],
	AC_DEFINE(HAVE_IMAGEMAGICK, 1, [ Define if you have imlib. ])
	report_have_libimlib="yes"
	AC_SUBST(IMLIB_CFLAGS)
	AC_SUBST(IMLIB_LIBS)
	, :)


AX_LIB_MYSQL([5.1.0])
if test "$mysql_version_check" = "1" ; then
	report_have_mysql="yes"
fi

AX_LIB_POSTGRESQL([9.0])
if test "$found_postgresql" = "yes"; then
	report_have_postgres="yes"
	if test $postgresql_version_number -ge 9002000 ; then
		AC_DEFINE([POSTGRESQL_HAVE_PQsetSingleRowMode], [1],
			[Define to 1 if PostgreSQL has function PQsetSingleRowMode])
	fi
fi

AX_LIB_SQLITE3
if test "$success" = "yes"; then
 	report_have_sqlite3="yes"
fi



AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_LDFLAGS)
AC_SUBST(MYSQL_VERSION)

AC_SUBST(SQLITE3_CFLAGS)
AC_SUBST(SQLITE3_LDFLAGS)
AC_SUBST(SQLITE3_VERSION)

AC_SUBST(POSTGRESQL_CFLAGS)
AC_SUBST(POSTGRESQL_LDFLAGS)
AC_SUBST(POSTGRESQL_VERSION)


AX_PATH_LIB_LDNS([1.6.3],
            AC_DEFINE(HAVE_LIBLDNS, 1, [ Define if you have ldns library from netlabs.nl. ])
            report_have_ldns="yes"
			AC_SUBST(LIBLDNS_CFLAGS)
			AC_SUBST(LIBLDNS_LIBS)
            ,
            )




dnl Check for nasm
AC_MSG_NOTICE(checking if we can use assembly)
use_asm="none"
report_have_nasm="no"
case $target in
	i?86*|x86*|amd64*)
		AC_CHECK_NASM(2.0.0,
			report_have_nasm="yes"
		)
		;;
esac

case $target in
	x86_64*|amd64*)
		AC_DEFINE(HAVE_AMD64,1,)
		;;
esac

if test $report_have_nasm = "yes"
then
	use_asm="nasm"
	AC_DEFINE(HAVE_NASM,1,)
	AC_DEFINE(HAVE_X86_ASSEMBLER,1,)
	HAVE_X86_ASSEMBLER="ASM_RELEASE	=	\$(ASM_OBJ_RELEASE)
ASM_DEBUG	=	\$(ASM_OBJ_DEBUG)
ASM_COVERAGE	=	\$(ASM_OBJ_COVERAGE)"
	AC_SUBST(HAVE_X86_ASSEMBLER)
	ASM=${NASM}
	ASMFLAGS=${NASMFLAGS}
	ASMDEBUGFLAGS=${NASMDEBUGFLAGS}
	AC_SUBST(ASM)
	AC_SUBST(ASMFLAGS)
	AC_SUBST(ASMDEBUGFLAGS)
fi

SRCDIR=`pwd`
AC_SUBST(SRCDIR)




# If we support __thread, that can speed up tcmalloc a bit.
# Note, however, that our code tickles a bug in gcc < 4.1.2
# involving TLS and -fPIC (which our libraries will use) on x86:
#   http://gcc.gnu.org/ml/gcc-bugs/2006-09/msg02275.html
AC_MSG_CHECKING([for __thread])
AC_LINK_IFELSE([AC_LANG_PROGRAM([#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)) && ((__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 1) || (__GNUC__ == 4 && __GNUC_MINOR__ == 1 && __GNUC_PATCHLEVEL__ < 2))
#error gcc has this bug: http://gcc.gnu.org/ml/gcc-bugs/2006-09/msg02275.html
#endif], [static __thread int p = 0])],
               [AC_DEFINE(HAVE_TLS, 1,
                      Define to 1 if compiler supports __thread)
                AC_MSG_RESULT([yes])],
               [AC_MSG_RESULT([no])])

# We also need to check if the kernel supports __thread, which requires uname()
AC_CHECK_DECLS(uname,,, [#include <sys/utsname.h>])




WINDOWS_WINSOCK_LIBS=""
case "$host_os" in
	*mingw32* )
		MINGW32=yes
		AC_DEFINE(MINGW32, 1, [Define if you compile on Windows MINGW32])
		#LIBS="$LIBS -lws2_32"
		AC_DEFINE(HAVE_LOCALTIME_R,1,[on mingw-w64 we always have localtime_r])
		AC_DEFINE(HAVE_GMTIME_R,1,[on mingw-w64 we always have gmtime_r])

		WINDOWS_WINSOCK_LIBS="-lws2_32"
  		;;
  	*mingw64*|msys )
		MINGW32=yes
		MINGW64=yes
		AC_DEFINE(MINGW32, 1, [Define if you compile on Windows MINGW32])
		AC_DEFINE(MINGW64, 1, [Define if you compile on Windows MINGW64])
		AC_DEFINE(HAVE_LOCALTIME_R,1,[on mingw-w64 we always have localtime_r])
		AC_DEFINE(HAVE_GMTIME_R,1,[on mingw-w64 we always have gmtime_r])

		WINDOWS_WINSOCK_LIBS="-lws2_32"
  		;;
	* )
		MINGW32=no
		;;
esac

AC_SUBST(WINDOWS_WINSOCK_LIBS)

case "${host_cpu}-${host_os}" in
	*solaris*)
		CFLAGS=`echo $CFLAGS | $SED 's/-I\/usr\/local\/include //g' | $SED 's/-I\/usr\/include //g' `
        ;;
esac;




AC_SUBST(report_have_zlib)
AC_SUBST(report_have_bzip2)
AC_SUBST(report_have_png)
AC_SUBST(report_have_jpeg)
AC_SUBST(report_have_freetype2)
AC_SUBST(report_have_lame)
AC_SUBST(report_have_libmad)
AC_SUBST(report_have_curl)
AC_SUBST(report_have_openssl)
AC_SUBST(report_have_pcre)
AC_SUBST(report_have_iconv)
AC_SUBST(report_have_mysql)
AC_SUBST(report_have_postgres)
AC_SUBST(report_have_sqlite3)
AC_SUBST(report_have_nasm)
AC_SUBST(report_have_pthreads)
AC_SUBST(report_have_ldns)
#AC_SUBST(report_have_libogg)
AC_SUBST(report_have_libvorbis)
AC_SUBST(report_have_libshout)
AC_SUBST(report_have_libimlib)
AC_SUBST(report_have_libtiff)
AC_SUBST(report_have_libmicrohttpd)
AC_SUBST(report_have_libidn)
AC_SUBST(report_have_libidn2)
AC_SUBST(report_have_libbind)
AC_SUBST(report_have_mpg123)


###############################################################################
# Output
###############################################################################

AC_OUTPUT


###############################################################################
# Report
###############################################################################

dnl report configuration
AC_MSG_RESULT([
******************************************************************************
** Configuration summary for $PACKAGE_NAME Version $PACKAGE_VERSION
** Target: $target_os        CPU: $target_cpu
**
** Byteorder: $report_byteorder
**   Size of char         $ac_cv_sizeof_char     Size of float        $ac_cv_sizeof_float
**   Size of short        $ac_cv_sizeof_short     Size of double       $ac_cv_sizeof_double
**   Size of int          $ac_cv_sizeof_int     Size of void*        $ac_cv_sizeof_voidp
**   Size of long         $ac_cv_sizeof_long     Size of wchar_t      $ac_cv_sizeof_wchar_t
**   Size of long long    $ac_cv_sizeof_long_long
**   Size of size_t       $ac_cv_sizeof_size_t
******************************************************************************
** Assembler:
**   Nasm (x86 assembler): $report_have_nasm
**
** Threads:                $report_have_pthreads
**
** Compression:
**   ZLIB library:         $report_have_zlib
**   BZIP2 library:        $report_have_bzip2
**
** Graphic:
**   BMP:                  yes (buildin)
**   GIF:                  yes (buildin)
**   TGA:                  yes (buildin)
**   PPM:                  yes (buildin)
**   PNG:                  $report_have_png
**   JPEG:                 $report_have_jpeg $jpeg_note
**   TIFF:                 $report_have_libtiff
**   Freetype 2.x library: $report_have_freetype2
**   ImageMagick:          $report_have_libimlib
**
** Multimedia:
**   Lame library:         $report_have_lame
**   mpg123 library:       $report_have_mpg123
**   vorbis library (ogg): $report_have_libvorbis
**   shout library:        $report_have_libshout
**   cdio library:         $report_have_libcdio
**
** Cryptography:
**   OpenSSL:              $report_have_openssl
**
** Internet:
**   Bind resolver lib:    $report_have_libbind
**   Curl library:         $report_have_curl
**   Libmicrohttpd:        $report_have_libmicrohttpd
**   ldns library:         $report_have_ldns
**   IDN1 library:         $report_have_libidn
**   IDN2 library:         $report_have_libidn2
**
** Databases:
**   SQLite3:              $report_have_sqlite3
**   MySQL:                $report_have_mysql
**   Postgres:             $report_have_postgres
**
** Other:
**   PCRE:                 $report_have_pcre
**   ICONV:                $report_have_iconv
**
** Testing:
**   Unittests:            $report_have_gtest
**   Codecoverage:         $report_codecoverage_enabled
******************************************************************************
])
