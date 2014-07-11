dnl AC_CHECK_LIBPJPEG([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_LIBJPEG],[
AC_MSG_CHECKING(if we should use libjpeg)

AC_ARG_WITH([libjpeg],
	[  --with-libjpeg[[=PATH]]     Prefix where libjpeg is installed (optional)],
	[libjpeg_prefix="$withval"],
	[libjpeg_prefix=""])
	if test "$libjpeg_prefix" != "no"
	then
		AC_MSG_RESULT(yes)
		am_save_CPPFLAGS="$CPPFLAGS"
		am_save_CFLAGS="$CFLAGS"
		am_save_LIBS="$LIBS"
		am_save_LDFLAGS="$LDFLAGS"
		if test "$libjpeg_prefix" = "yes" -o "$libjpeg_prefix" = "auto"
		then
			PKG_CHECK_MODULES([JPEG], [libjpeg],
				have_libjpeg="yes"
				LIBS="$JPEG_LIBS"
				CFLAGS="$JPEG_CFLAGS"
				CPPFLAGS="$JPEG_CFLAGS"
				,
				have_libjpeg="no"
				JPEG_LIBS="-ljpeg"
			)
		else
			LIBS="-L$libjpeg_prefix/lib"
			CFLAGS="-I$libjpeg_prefix/include"
			CPPFLAGS="-I$libjpeg_prefix/include"
			LDFLAGS="-L$libjpeg_prefix/lib"
			JPEG_LIBS="-L$libjpeg_prefix/lib -ljpeg"
			JPEG_CFLAGS="-I$libjpeg_prefix/include"
		fi
		
		AC_CHECK_LIB(jpeg,jpeg_finish_decompress,
			have_libjpeg="yes"
			,
			,
		)
		AC_CHECK_HEADERS([jpeglib.h],,have_libjpeg="no")
		LIBS="$am_save_LIBS"
		LDFLAGS="$am_save_LDFLAGS"
		CFLAGS="$am_save_CFLAGS"
		CPPFLAGS="$am_save_CPPFLAGS"
	else
		AC_MSG_RESULT(no)
	fi
	if test $have_libjpeg = "yes"
	then
		dnl echo "JPEG_LIBS=$JPEG_LIBS"
		dnl echo "JPEG_CFLAGS=$JPEG_CFLAGS"
		ifelse([$2], , :, [$2])
	else
		JPEG_LIBS=""
		JPEG_CFLAGS=""
		ifelse([$3], , :, [$3])
	fi

])