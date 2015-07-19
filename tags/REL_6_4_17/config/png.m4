dnl AC_CHECK_LIBPNG([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_LIBPNG],[

AC_ARG_WITH([png],
	[  --with-png[[=PATH]]     Prefix where libpng is installed (optional)],
	[png_prefix="$withval"],
	[png_prefix="no"])

	pngconfig=""
	if test "$png_prefix" != "yes" && test "$png_prefix" != "no"
	then
		pngconfig="$png_prefix/bin/libpng-config"
	elif test "$png_prefix" != "no"
	then
		AC_PATH_PROG(pngconfig,libpng-config)
	fi
	if test [ -z "$pngconfig" ]
	then
		AC_MSG_CHECKING(for libpng)
		AC_MSG_RESULT(no)
		ifelse([$3], , :, [$3])
	else
		AC_MSG_CHECKING(png libraries)
		PNG_LIBS=`${pngconfig} --libs`
		AC_MSG_RESULT($PNG_LIBS)
		AC_MSG_CHECKING(png includes)
		PNG_CFLAGS=`${pngconfig} --cflags`
		AC_MSG_RESULT($PNG_CFLAGS)
		AC_MSG_CHECKING(for libpng)
		AC_MSG_RESULT(yes)
		ifelse([$2], , :, [$2])
	fi

])

