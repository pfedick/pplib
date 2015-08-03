dnl AC_CHECK_SQLITE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_SQLITE],[
AC_MSG_CHECKING(if SQLite should be used)

AC_ARG_WITH([sqlite],
	[  --with-sqlite[[=PATH]]   Prefix where SQLite is installed (optional)],
	[sqlite_prefix="$withval"],
	[sqlite_prefix="auto"])

dnl echo "sqlite_prefix=$sqlite_prefix"
have_sqlite="no"
if test "$sqlite_prefix" != "no"
then
	AC_MSG_RESULT(yes)
	if test "$sqlite_prefix" = "yes" -o "$sqlite_prefix" = "auto"
	then
		PKG_CHECK_MODULES([SQLITE], [sqlite3],
			have_sqlite="yes"
			,
			have_sqlite="no"
		)
	else
		my_save_LIBS="$LIBS"
		my_save_CFLAGS="$CFLAGS"
		my_save_LDFLAGS="$LDFLAGS"
		LDFLAGS="$LDFLAGS -L/usr/local/lib"
		CFLAGS="$CFLAGS -I$sqlite_prefix/include"
		AC_CHECK_LIB([sqlite3],[sqlite3_open],
            SQLITE_LIBS="-L$sqlite_prefix/lib -lsqlite3"
            SQLITE_CFLAGS="-I$sqlite_prefix/include"
			have_sqlite="yes"
		)
		LIBS="$my_save_LIBS"
		CFLAGS="$my_save_CFLAGS"
		LDFLAGS="$my_save_LDFLAGS"
	fi
else
	AC_MSG_RESULT(no)
fi

if test $have_sqlite = "yes"
then
	ifelse([$2], , :, [$2])
else
	ifelse([$3], , :, [$3])
fi

])

