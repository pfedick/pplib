dnl AX_PATH_LIB_PCRE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AX_PATH_LIB_PCRE],[dnl
AC_MSG_CHECKING([lib pcre])

AC_ARG_WITH([pcre],
	[  --with-pcre[[=PATH]]      Prefix where pcre is installed (optional)],
	[pcre_prefix="$withval"],
	[pcre_prefix="no"])

if test "$pcre_prefix" != "no"
then
	if test "$pcre_prefix" != "yes"
	then
		pcreconfig="$pcre_prefix/bin/pcre-config"
	else
		AC_PATH_PROG(pcreconfig,pcre-config)
	fi
	if test [ ! -x "$pcreconfig" ] ; then
		pcreconfig=""
	fi
	
	if test [ -z "$pcreconfig" ]
	then
		AC_MSG_RESULT(no)
		ifelse([$3], , :, [$3])
	else
		AC_MSG_RESULT(yes: $pcreconfig)
		AC_MSG_CHECKING(pcre libraries)
		PCRE_LIBS=`${pcreconfig} --libs`
		AC_MSG_RESULT($PCRE_LIBS)
		AC_MSG_CHECKING(pcre includes)
		PCRE_CFLAGS=`${pcreconfig} --cflags`
		AC_MSG_RESULT($PCRE_CFLAGS)
		ifelse([$2], , :, [$2])
	fi
else
	AC_MSG_RESULT(not configured)
fi
])

