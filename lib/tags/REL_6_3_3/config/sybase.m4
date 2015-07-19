dnl AC_CHECK_SYBASE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_SYBASE],[
AC_MSG_CHECKING(for Sybase Open Client-Library)

AC_ARG_WITH([sybase],
	[  --with-sybase[[=PATH]]    Prefix where freetds is installed (optional)],
	[sybase_prefix="$withval"],
	[sybase_prefix="no"])

if test "$sybase_prefix" != "no"
then

	if test "$sybase_prefix" != "yes"
	then
		tsql="$sybase_prefix/bin/tsql"
	else
		AC_PATH_PROG(tsql,tsql)
	fi
	isfreetds="no"
	if test [ -n "$tsql" ]
	then
		if test -x "$tsql"
		then
			isfreetds="yes"
		fi
	fi
	have_sybase="yes"
	sybprefix=`echo $tsql | sed 's!/bin/tsql!!'`
	if ! test -f "$sybprefix/include/sybdb.h"
	then
		have_sybase="no"
	fi
	if ! test -f "$sybprefix/lib/libsybdb.a"
	then
		have_sybase="no"
	fi

	SYBASE_LIBS="-L$sybprefix/lib"

	if test -f "$sybprefix/lib/libsybct_r.a" ; then
		SYBASE_LIBS="$SYBASE_LIBS -lsybct_r"
	elif test -f "$sybprefix/lib/libct_r.a" ; then
		SYBASE_LIBS="$SYBASE_LIBS -lct_r"
	elif test -f "$sybprefix/lib/libct.a" ; then
		SYBASE_LIBS="$SYBASE_LIBS -lct"

	fi

	if test -f "$sybprefix/lib/libsybcs_r.a" ; then
		SYBASE_LIBS="$SYBASE_LIBS -lsybcs_r"
	elif test -f "$sybprefix/lib/libcs_r.a" ; then
		SYBASE_LIBS="$SYBASE_LIBS -lcs_r"
	fi

	if test -f "$sybprefix/lib/libsybintl_r.a" ; then
		SYBASE_LIBS="$SYBASE_LIBS -lsybintl_r"
	elif test -f "$sybprefix/lib/libintl_r.a" ; then
		SYBASE_LIBS="$SYBASE_LIBS -lintl_r"
	fi

	
	if test -f "$sybprefix/lib/libsybtcl_r.a"
	then
		SYBASE_LIBS="$SYBASE_LIBS -lsybtcl_r"
	fi
	if test -f "$sybprefix/lib/libsybcomn_r.a"
	then
		SYBASE_LIBS="$SYBASE_LIBS -lsybcomn_r"
	elif test -f "$sybprefix/lib/libcomn_r.a"
	then
		SYBASE_LIBS="$SYBASE_LIBS -lcomn_r"
	fi
	if test "$isfreetds" = "yes"
	then
		msg="yes, via FreeTDS"
		AC_DEFINE(HAVE_FREETDS, 1, [ Define if you have FreeTDS Library. ])
		
	else
		msg="yes, native Sybase Library"
		SYBASE_LIBS="$SYBASE_LIBS -lpthread"
	fi
	if test "$have_sybase" = "yes"
	then
		AC_MSG_RESULT($msg)
		SYBASE_CFLAGS="-I$sybprefix/include"
		AC_MSG_CHECKING(sybase libraries)
		AC_MSG_RESULT($SYBASE_LIBS)
		AC_MSG_CHECKING(sybase includes)
		AC_MSG_RESULT($SYBASE_CFLAGS)
		ifelse([$2], , :, [$2])
	else
		ifelse([$3], , :, [$3])
	fi
else
	AC_MSG_RESULT(no)
fi
])

