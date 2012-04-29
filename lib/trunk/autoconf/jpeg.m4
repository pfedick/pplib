dnl AC_CHECK_LIBPJPEG([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_LIBJPEG],[
AC_MSG_CHECKING(for libjpeg)

AC_ARG_WITH([jpeg],
	[  --with-jpeg[[=PATH]]     Prefix where libjpeg is installed (optional)],
	[jpeg_prefix="$withval"],
	[jpeg_prefix=""])
	if test "$jpeg_prefix" != "no"
	then
		AC_MSG_RESULT(yes)
		am_save_CPPFLAGS="$CPPFLAGS"
		am_save_CFLAGS="$CFLAGS"
		am_save_LIBS="$LIBS"
		am_save_LDFLAGS="$LDFLAGS"

		PKG_CHECK_MODULES([JPEG], [libjpeg >= 6.0.0],
			ifelse([$2], , :, [$2])
			,
			AC_SEARCH_LIBS( [jpeg_finish_decompress],[jpeg],
				dnl echo "LIBS=$LIBS"
				dnl echo "LDFLAGS=$LDFLAGS"
				dnl echo "CFLAGS=$CFLAGS"
				dnl echo "CPPFLAGS=$CPPFLAGS"
			
				JPEG_LIBS="$LIBS"
				JPEG_CFLAGS=""
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