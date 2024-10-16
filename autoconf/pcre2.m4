dnl AC_CHECK_PCRE2([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_PCRE2],[
AC_MSG_CHECKING(if we should use pcre2)

AC_ARG_WITH([pcre2],
	[  --with-pcre2[[=PATH]]     Prefix where libpcre2 is installed (optional)],
	[libpcre2_prefix="$withval"],
	[libpcre2_prefix="auto"])

	if test "$libpcre2_prefix" != "no"
	then
		AC_MSG_RESULT(yes)
		am_save_CFLAGS="$CFLAGS"
		am_save_CPPFLAGS="$CPPFLAGS"
		am_save_LIBS="$LIBS"
		am_save_LDFLAGS="$LDFLAGS"
		am_pcrebits=""
		if test ${ac_cv_sizeof_wchar_t} -eq 4
		then
			am_pcrebits="32"
		elif test ${ac_cv_sizeof_wchar_t} -eq 2
		then
			am_pcrebits="16"
		fi
		

		if test "$libpcre2_prefix" = "yes" -o "$libpcre2_prefix" = "auto"
		then
			AC_PATH_PROG(pcre2config,pcre2-config)
			PKG_CHECK_MODULES([PCRE2_8], [libpcre2-8],
				have_libpcre2="yes"
				PCRE2_LIBS="$PCRE2_8_LIBS"
				PCRE2_CFLAGS="$PCRE2_8_CFLAGS"
				AC_DEFINE(HAVE_PCRE2_CHAR)
				,
				have_libpcre2="no"
				PCRE2_LIBS=""
				PCRE2_CFLAGS=""
				
				if test [ -x "$pcre2config" ]
				then
					have_libpng="yes"
					PCRE2_LIBS=`$pcre2config --libs8`
					PCRE2_CFLAGS=`$pcre2config --cflags`
					AC_DEFINE(HAVE_PCRE2_CHAR)
				fi
			)
			PKG_CHECK_MODULES([PCRE2_WIDE], [libpcre2-$am_pcrebits],
				PCRE2_LIBS="$PCRE2_LIBS $PCRE2_WIDE_LIBS"
				PCRE2_CFLAGS="$PCRE2_CFLAGS $PCRE2_WIDE_CFLAGS"
				AC_DEFINE(HAVE_PCRE2_WCHAR)
				,
				have_libpcre2="no"
				PCRE2_LIBS=""
				PCRE2_CFLAGS=""
				
				if test [ -x "$pcre2config" ]
				then
					have_libpng="yes"
					PCRE2_LIBS=`$PCRE2_LIBS $pcre2config --libs$am_pcrebits`
					AC_DEFINE(HAVE_PCRE2_WCHAR)
				fi

			)
		else
			LIBS="-L$libpng_prefix/lib"
			CFLAGS="-I$libpng_prefix/include"
			CPPFLAGS="-I$libpng_prefix/include"
			LDFLAGS="-L$libpng_prefix/lib"
			PCRE2_LIBS="-L$libpcre2_prefix/lib -lpcre2-8"
			PCRE2_CFLAGS="-I$libpcre2_prefix/include"
			AC_DEFINE(HAVE_PCRE2_CHAR)
		fi
		

		LIBS="$am_save_LIBS"
		LDFLAGS="$am_save_LDFLAGS"
		CFLAGS="$am_save_CFLAGS"
		CPPFLAGS="$am_save_CPPFLAGS"
	else
		AC_MSG_RESULT(no)
	fi
	if test $have_libpcre2 = "yes"
	then
		dnl echo "PCRE2_LIBS=$PCRE2_LIBS"
		dnl echo "PCRE2_CFLAGS=$PCRE2_CFLAGS"
		ifelse([$1], , :, [$1])
	else
		PCRE2_LIBS=""
		PCRE2_CFLAGS=""
		ifelse([$2], , :, [$2])
	fi
	AC_SUBST(PCRE2_LIBS)
	AC_SUBST(PCRE2_CFLAGS)

])



