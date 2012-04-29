dnl AC_CHECK_LIBPNG([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_LIBPNG],[
AC_MSG_CHECKING(if we should use libpng)

AC_ARG_WITH([libpng],
	[  --with-libpng[[=PATH]]     Prefix where libpng is installed (optional)],
	[libpng_prefix="$withval"],
	[libpng_prefix=""])

	if test "$libpng_prefix" != "no"
	then
		AC_MSG_RESULT(yes)
		am_save_CPPFLAGS="$CPPFLAGS"
		am_save_CFLAGS="$CFLAGS"
		am_save_LIBS="$LIBS"
		am_save_LDFLAGS="$LDFLAGS"
		if test -n "$libpng_prefix" && test "$libpng_prefix" != "yes"
		then
			LIBS="$LIBS -L$libpng_prefix/lib"
			CFLAGS="$LIBS -I$libpng_prefix/include"
			AC_SEARCH_LIBS( [png_sig_cmp],[png],
				PNG_LIBS="-L$libpng_prefix/lib"
				PNG_CFLAGS="-I$libpng_prefix/include"
				ifelse([$2], , :, [$2])
  				,
				ifelse([$3], , :, [$3])
			:)
		else
			PKG_CHECK_EXISTS([libpng],
				PKG_CHECK_MODULES([PNG], [libpng >= $1],
					ifelse([$2], , :, [$2])
					,
					ifelse([$3], , :, [$3])
				)
				,
				AC_SEARCH_LIBS( [png_sig_cmp],[png],
					PNG_LIBS="$LIBS"
					PNG_CFLAGS=""
					ifelse([$2], , :, [$2])
  					,
					ifelse([$3], , :, [$3])
				:)
			)
		fi
		CPPFLAGS="$am_save_CPPFLAGS"
		LIBS="$am_save_LIBS"
		LDFLAGS="$am_save_LDFLAGS"
		CFLAGS="$am_save_CFLAGS"	
	else
		AC_MSG_RESULT(no)
		ifelse([$3], , :, [$3])
	fi
])


