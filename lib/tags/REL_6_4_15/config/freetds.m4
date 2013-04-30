dnl AC_CHECK_FREETDS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_FREETDS],[
AC_MSG_CHECKING(if FreeTDS should be used)

AC_ARG_WITH([freetds],
	[  --with-freetds[[=PATH]]   Prefix where freetds is installed (optional)],
	[freetds_prefix="$withval"],
	[freetds_prefix="auto"])

if test "$freetds_prefix" != "no"
then
	AC_MSG_RESULT(yes)
	my_save_LIBS="$LIBS"
	my_save_CFLAGS="$CFLAGS"
	my_save_LDFLAGS="$LDFLAGS"
	
	
	if test "$freetds_prefix" = "yes" -o "$freetds_prefix" = "auto"
	then
		PKG_CHECK_MODULES([FreeTDS], [freedts],	have_freetds="yes", have_freetds="no")
	else
		LIBS="-L$freetds_prefix/lib"
		CFLAGS="-I$freetds_prefix/include"
		FREETDS_LIBS="-L$freetds_prefix/lib"
		FREETDS_CFLAGS="-I$freetds_prefix/include"
	fi
	
	if test "$have_freetds" != "yes"
	then
		dnl echo "FreeTDS ist nicht in autoconf, manuelle suche..."
		AC_CHECK_LIB(rt,clock_gettime,
			FREETDS_LIBS="$FREETDS_LIBS -lrt "
			LIBS="$LIBS -lrt"
		)
		AC_CHECK_LIB(ct,ct_connect,
			have_freetds="yes"
			FREETDS_LIBS="-lct $FREETDS_LIBS "
		)
		AC_CHECK_HEADERS([ctpublic.h],,have_freetds="no")
	fi
	LIBS="$my_save_LIBS"
	CFLAGS="$my_save_CFLAGS"
	LDFLAGS="$my_save_LDFLAGS"
else
	AC_MSG_RESULT(no)
fi

if test $have_freetds = "yes"
then
	echo "FREETDS_LIBS=$FREETDS_LIBS"
	echo "FREETDS_CFLAGS=$FREETDS_CFLAGS"
	
	ifelse([$2], , :, [$2])
else
	FREETDS_LIBS=""
	FREETDS_CFLAGS=""
	ifelse([$3], , :, [$3])
fi


])

