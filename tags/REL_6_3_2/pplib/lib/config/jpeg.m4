dnl AC_CHECK_LIBPJPEG([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_LIBJPEG],[
AC_MSG_CHECKING(for libjpeg)

AC_ARG_WITH([jpeg],
	[  --with-jpeg[[=PATH]]     Prefix where libjpeg is installed (optional)],
	[jpeg_prefix="$withval"],
	[jpeg_prefix="no"])

	AC_MSG_CHECKING(for libpng)
	if test "$jpeg_prefix" != "no"
	then

		am_save_CPPFLAGS="$CPPFLAGS"
		am_save_LIBS="$LIBS"
		am_save_LDFLAGS="$LDFLAGS"

		LIBS="$LIBS -L/usr/local/lib -L/usr/lib"
		CFLAGS="$CFLAGS -I/usr/local/include -I/usr/include"

		AC_CHECK_LIB(jpeg,jpeg_finish_decompress,
			JPEG_LIBS="-ljpeg"
			JPEG_CFLAGS=""
			AC_MSG_RESULT(yes)
			ifelse([$2], , :, [$2])
  			,
  			AC_MSG_RESULT(no)
			ifelse([$3], , :, [$3])
		:)
		CPPFLAGS="$am_save_CPPFLAGS"
		LIBS="$am_save_LIBS"
		LDFLAGS="$am_save_LDFLAGS"
	else
		AC_MSG_RESULT(no)
		ifelse([$3], , :, [$3])
	fi
])