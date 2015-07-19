dnl AC_CHECK_FREETDS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_FREETDS],[
AC_MSG_CHECKING(for FreeTDS)

AC_ARG_WITH([freetds],
	[  --with-freetds[[=PATH]]   Prefix where freetds is installed (optional)],
	[freetds_prefix="$withval"],
	[freetds_prefix="no"])

if test "$freetds_prefix" != "no"
then

	if test "$freetds_prefix" != "yes"
	then
		tsql="$freetds_prefix/bin/tsql"
	else
		AC_PATH_PROG(tsql,tsql)
	fi
	if test [ -z "$tsql" ]
	then
		ifelse([$3], , :, [$3])
	else
		have_tds="yes"
		tdsprefix=`echo $tsql | sed 's!/bin/tsql!!'`
		if ! test -f "$tdsprefix/include/tds.h"
		then
			have_tds="no"
		fi
		if test -f "$tdsprefix/lib/libtds.h"
		then
			have_tds="no"
		fi
		if test "$have_tds" == "yes"
			AC_MSG_RESULT(yes)
			# most systems require the program be linked with librt library to use
			# the function clock_gettime 
			my_save_LIBS="$LIBS"
			LIBS=""
			AC_CHECK_LIB(rt,clock_gettime)
			LIBRT=$LIBS
			LIBS="$my_save_LIBS"
			AC_SUBST(LIBRT)
			AC_CHECK_FUNCS(clock_gettime)
		
			FREETDS_LIBS="-L$tdsprefix/lib -ltds $LIBRT"
			FREETDS_CFLAGS="-I$tdsprefix/include"
			AC_MSG_CHECKING(freetds libraries)
			AC_MSG_RESULT($FREETDS_LIBS)
			AC_MSG_CHECKING(freetds includes)
			AC_MSG_RESULT($FREETDS_CFLAGS)
			ifelse([$2], , :, [$2])
		then
			ifelse([$3], , :, [$3])
		fi
	fi
else
	AC_MSG_RESULT(no)
fi
])

