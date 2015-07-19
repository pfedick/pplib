dnl AX_PATH_LIB_IMLIB([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AX_PATH_LIB_IMLIB],[dnl
AC_MSG_CHECKING([lib ImageMagick])

AC_ARG_WITH([imlib],
	[  --with-imlib[[=PATH]]      Prefix where ImageMagick is installed (default=autodetected)],
	[imlib_prefix="$withval"],
	[imlib_prefix=""])


if test "$imlib_prefix" != "no"
then
	save_path=$PATH
	if test -n "$imlib_prefix"
	then
		if test "$imlib_prefix" != "yes"
		then
			PATH=$imlib_prefix:$PATH
			export PATH
		fi
	fi
	echo "Suche nach imlib, imlib_prefix=$imlib_prefix, PATH: $PATH"
	AC_PATH_PROG(imlibconfig,MagickWand-config)
	PATH=$save_path
	echo "imlibconfig=$imlibconfig"
	if test [ ! -x "$imlibconfig" ] ; then
		imlibconfig=""
	fi
	
	if test [ -z "$imlibconfig" ]
	then
		AC_MSG_RESULT(no)
		ifelse([$3], , :, [$3])
	else
		AC_MSG_RESULT(yes: $imlibconfig)
		AC_MSG_CHECKING(imlib libraries)
		IMLIB_LIBS=`${imlibconfig} --libs`
		AC_MSG_RESULT($IMLIB_LIBS)
		AC_MSG_CHECKING(imlib includes)
		IMLIB_CFLAGS=`${imlibconfig} --cflags`
		AC_MSG_RESULT($IMLIB_CFLAGS)
		ifelse([$2], , :, [$2])
	fi
else
	AC_MSG_RESULT(not configured)
fi
])

