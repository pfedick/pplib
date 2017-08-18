dnl AC_CHECK_LIBTIFF([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AC_CHECK_LIBTIFF],[
AC_MSG_CHECKING(for libtiff)

AC_ARG_WITH([libtiff],
	[  --with-libtiff[[=PATH]]     Prefix where libtiff is installed (optional)],
	[libtiff_prefix="$withval"],
	[libtiff_prefix="no"])

	if test "$libtiff_prefix" != "no"
	then
		if test "$libtiff_prefix" != "yes"
		then
			intLIBTIFF_LIBS="-L $libtiff_prefix/lib -ltiff"
			intLIBTIFF_CFLAGS="-I $libtiff_prefix/include"
		else
			intLIBTIFF_LIBS="-ltiff"
			intLIBTIFF_CFLAGS=""
		fi
		
		am_save_CPPFLAGS="$CPPFLAGS"
		am_save_LIBS="$LIBS"
		am_save_LDFLAGS="$LDFLAGS"
		am_save_CFLAGS="$CFLAGS"
		
		AC_CHECK_LIB(lzma,lzma_code,
			intLIBTIFF_LIBS="$intLIBTIFF_LIBS -llzma"
			AC_MSG_RESULT(yes)
			,
			AC_MSG_RESULT(no)
		)
				
		LIBS="$LIBS $intLIBTIFF_LIBS $JPEG_LIBS -lz"
		CFLAGS="$CFLAGS $intLIBTIFF_CFLAGS $JPEG_CFLAGS"
				
		AC_CHECK_HEADERS([tiffio.h],
			AC_CHECK_LIB(tiff,TIFFClientOpen,
				TIFF_LIBS=$intLIBTIFF_LIBS
				TIFF_CFLAGS=$intLIBTIFF_CFLAGS
				AC_MSG_RESULT(yes)
				ifelse([$2], , :, [$2])
				,
				AC_MSG_RESULT(no)
				ifelse([$3], , :, [$3])
			:)
		,
			AC_MSG_RESULT(no)
			ifelse([$3], , :, [$3])
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

