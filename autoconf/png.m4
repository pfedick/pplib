dnl AC_CHECK_LIBPNG([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_LIBPNG],[
AC_MSG_CHECKING(if we should use libpng)

AC_ARG_WITH([libpng],
	[  --with-libpng[[=PATH]]     Prefix where libpng is installed (optional)],
	[libpng_prefix="$withval"],
	[libpng_prefix="auto"])

	if test "$libpng_prefix" != "no"
	then
		AC_MSG_RESULT(yes)
		am_save_CFLAGS="$CFLAGS"
		am_save_CPPFLAGS="$CPPFLAGS"
		am_save_LIBS="$LIBS"
		am_save_LDFLAGS="$LDFLAGS"
		if test "$libpng_prefix" = "yes" -o "$libpng_prefix" = "auto"
		then
			AC_PATH_PROG(libpngconfig,libpng-config)
			PKG_CHECK_MODULES([PNG], [libpng],
				have_libpng="yes"
				LIBS="$PNG_LIBS"
				CFLAGS="$PNG_CFLAGS"
				CPPFLAGS="$PNG_CFLAGS"
				,
				have_libpng="no"
				PNG_LIBS="-lpng -lz -lm"

				
				if test [ -x "$libpngconfig" ]
				then
					have_libpng="yes"
					PNG_LIBS=`$libpngconfig --ldflags`
					PNG_LIBS="$PNG_LIBS -lz -lm"
					PNG_CFLAGS=`$libpngconfig --cflags`
					LIBS="$PNG_LIBS"
					CFLAGS="$PNG_CFLAGS"
					CPPFLAGS="$PNG_CFLAGS"
				fi

			)
		else
			LIBS="-L$libpng_prefix/lib"
			CFLAGS="-I$libpng_prefix/include"
			CPPFLAGS="-I$libpng_prefix/include"
			LDFLAGS="-L$libpng_prefix/lib"
			PNG_LIBS="-L$libpng_prefix/lib -lpng -lz -lm"
			PNG_CFLAGS="-I$libpng_prefix/include"
		fi
		
		AC_CHECK_LIB(png,png_sig_cmp,
			have_libpng="yes"
			,
			,
			-lz -lm
		)
		AC_CHECK_HEADERS([png.h],,have_libpng="no")

		LIBS="$am_save_LIBS"
		LDFLAGS="$am_save_LDFLAGS"
		CFLAGS="$am_save_CFLAGS"
		CPPFLAGS="$am_save_CPPFLAGS"
	else
		AC_MSG_RESULT(no)
	fi
	if test $have_libpng = "yes"
	then
		dnl echo "PNG_LIBS=$PNG_LIBS"
		dnl echo "PNG_CFLAGS=$PNG_CFLAGS"
		ifelse([$2], , :, [$2])
	else
		PNG_LIBS=""
		PNG_CFLAGS=""
		ifelse([$3], , :, [$3])
	fi

])



