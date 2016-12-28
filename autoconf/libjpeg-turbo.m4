dnl AC_CHECK_LIBJPEGTURBO([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_LIBJPEGTURBO],[
AC_MSG_CHECKING(for libjpegturbo)

AC_ARG_WITH([libjpegturbo],
	[  --with-libjpegturbo[[=PATH]]     Prefix where libjpeg-turbo is installed (optional)],
	[libjpegturbo_prefix="$withval"],
	[libjpegturbo_prefix=""])
	if test "$libjpegturbo_prefix" != "no"
	then
		AC_MSG_RESULT(yes)
		am_save_CPPFLAGS="$CPPFLAGS"
		am_save_CFLAGS="$CFLAGS"
		am_save_LIBS="$LIBS"
		am_save_LDFLAGS="$LDFLAGS"
		LIBS=""

		PKG_CHECK_MODULES([JPEG], [libturbojpeg >= $1],
			ifelse([$2], , :, [$2])
			,
			AC_SEARCH_LIBS( [tjInitDecompress],[turbojpeg],
				dnl echo "LIBS=$LIBS"
				dnl echo "LDFLAGS=$LDFLAGS"
				dnl echo "CFLAGS=$CFLAGS"
				dnl echo "CPPFLAGS=$CPPFLAGS"
			
				JPEG_TURBO_LIBS="$LIBS"
				JPEG_TURBO_CFLAGS=""
				ifelse([$2], , :, [$2])
  				,
				ifelse([$3], , :, [$3])
			:)
		)
		CPPFLAGS="$am_save_CPPFLAGS"
		LIBS="$am_save_LIBS"
		LDFLAGS="$am_save_LDFLAGS"
		CFLAGS="$am_save_CFLAGS"
	else
		AC_MSG_RESULT(no)
		ifelse([$3], , :, [$3])
	fi
])