#pragma once
#ifndef PPL7_VISUALC_CONFIG
#define PPL7_VISUALC_CONFIG

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// define HAVE_STRING_H 0
#define HAVE_STRING_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDARG_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_CRTDEFS_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_LIMITS_H 1
#define HAVE_CTYPE_H 1
#define HAVE_WCHAR_H 1
#define HAVE_WCTYPE_H 1
#define HAVE_LOCALE_H 1
#define HAVE_ERRNO_H 1
//#define HAVE_WIDEC_H 0

#define HAVE_STRNCPY_S 1
#define HAVE_WCSNCPY_S 1
#define HAVE_WCSTOMBS_S 1
#define HAVE_MBSTOWCS_S 1




#define ppluint8 unsigned char
#define pplint8 signed char
#define ppluint16 unsigned short
#define pplint16 signed short
#define ppluint32 unsigned int
#define pplint32 signed int
#define ppluint64 unsigned long long
#define pplint64 signed long long

#define ppliptr ppluint64

#define pplfloat	float
#define ppldouble	double
#define pplbool		bool
#define pplchar		char
#define ppluchar	unsigned char


#define ssize_t long long
#define mode_t int


#define ICONV_UNICODE "UTF-16LE"

#endif