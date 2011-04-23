dnl AX_PATH_LIB_PCRE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AX_PATH_LIB_PCRE],[dnl
AC_MSG_CHECKING([lib pcre])

AC_ARG_WITH([pcre],
	[  --with-pcre[[=PATH]]      Prefix where pcre is installed (default=autodetected)],
	[pcre_prefix="$withval"],
	[pcre_prefix=""])


if test "$pcre_prefix" != "no"
then
	save_path=$PATH
	if test -n "$pcre_prefix"
	then
		if test "$pcre_prefix" != "yes"
		then
			PATH=$pcre_prefix:$PATH
			export PATH
		fi
	fi
	echo "Suche nach pcre, pcre_prefix=$pcre_prefix, PATH: $PATH"
	AC_PATH_PROG(pcreconfig,pcre-config)
	PATH=$save_path
	echo "pcreconfig=$pcreconfig"
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

