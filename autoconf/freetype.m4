
dnl AC_CHECK_FT2([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AC_CHECK_FT2],[
AC_MSG_CHECKING(if we should use freetype)
AC_ARG_WITH([freetype2],
	[  --with-freetype2[[=PATH]]     Prefix where freetype2 is installed (optional)],
	[freetype_prefix="$withval"],
	[freetype_prefix="auto"])
	if test "$freetype2_prefix" != "no"
	then
		AC_MSG_RESULT(yes)
		if test "$freetype_prefix" != "yes"
		then
			intFREETYPE_LIBS="-L $freetype_prefix/lib -lfreetype"
			intFREETYPE_CFLAGS="-I $freetype_prefix/include/freetype2"
		else
			intFREETYPE_LIBS="-lfreetype"
			intFREETYPE_CFLAGS="-Ifreetype2"
		fi

		am_save_CPPFLAGS="$CPPFLAGS"
		am_save_LIBS="$LIBS"
		am_save_LDFLAGS="$LDFLAGS"
		am_save_CFLAGS="$CFLAGS"

		LIBS="$LIBS $intFREETYPE_LIBS"
		CFLAGS="$CFLAGS $intFREETYPE_CFLAGS"
		AC_CHECK_HEADER([ft2build.h],

			AC_CHECK_LIB(freetype,FT_Init_FreeType,
				FT2_LIBS=$intFREETYPE_LIBS
				FT2_CFLAGS=$intFREETYPE_CFLAGS

				ifelse([$2], , :, [$2])
				,
				ifelse([$3], , :, [$3])
			:)
		,
			ifelse([$3], , :, [$3])
		)
		CPPFLAGS="$am_save_CPPFLAGS"
		LIBS="$am_save_LIBS"
		LDFLAGS="$am_save_LDFLAGS"
		CFLAGS="$am_save_CFLAGS"
    AC_SUBST([FT2_CFLAGS])
    AC_SUBST([FT2_LIBS])

	else
		AC_MSG_RESULT(no)
		ifelse([$3], , :, [$3])
	fi
])
