dnl AX_CHECK_LIBCDDB([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl Autor: Patrick Fedick <patrick@pfp.de>
dnl
dnl If library is found, it sets the following defines:
dnl    HAVE_LIBCDDB
dnl    LIBCDDB_CFLAGS
dnl    LIBCDDB_LDFLAGS

AC_DEFUN([AX_CHECK_LIBCDDB],[
AC_MSG_CHECKING(if we should use libcddb)

AC_ARG_WITH([libcddb],
	[  --with-libcddb[[=PATH]]   support accesing audio cds (optional)],
	[libcddb_prefix="$withval"],
	[libcddb_prefix="auto"])

	have_libcddb="no"

	if test "$libcddb_prefix" != "no"
	then
		AC_MSG_RESULT(yes)
		
		am_save_CPPFLAGS="$CPPFLAGS"
		am_save_LIBS="$LIBS"
		am_save_LDFLAGS="$LDFLAGS"
		
		if test -n "$libcddb_prefix" -a "$libcddb_prefix" != "yes" -a "$libcddb_prefix" != "auto"
		then
		    LIBCDDB_CFLAGS="-I$with_libcddb/include"
		    LIBCDDB_LDFLAGS="-L$with_libcddb/lib -lcddb" 
		else
			AC_PATH_PROG([PKGCONFIG], [pkg-config], [no])
			if test "$PKGCONFIG" != "no" && `$PKGCONFIG --exists libcddb`
			then
				LIBCDDB_CFLAGS=`$PKGCONFIG --cflags libcddb`
		    		LIBCDDB_LDFLAGS=`$PKGCONFIG --libs libcddb`
		    	else 
		    		PKG_CHECK_MODULES([libcddb], [libcddb],
					LIBCDDB_CFLAGS=$libcddb_CFLAGS
					LIBCDDB_LDFLAGS=$libcddb_LIBS
				,
					LIBCDDB_LDFLAGS="-lcdio"
				)
			fi
		fi
		
		CFLAGS="$CFLAGS $LIBCDDB_CFLAGS"
		LDFLAGS="$LDFLAGS $LIBCDDB_LDFLAGS"
		AC_MSG_CHECKING(if we can link against libcddb)
		AC_TRY_LINK([
			#include <stdio.h>
			#include <cddb/cddb.h>
			],[
			int main(int argc, char **argv) {
				cddb_conn_t *conn=cddb_new();
				cddb_disc_t *disc = cddb_disc_new();
				int matches = cddb_query(conn, disc);
			}
			],
				AC_MSG_RESULT(yes)
				have_libcddb="yes"
			,
				AC_MSG_RESULT(no)
			)
				
			
		CPPFLAGS=$am_save_CPPFLAGS
		LIBS=$am_save_LIBS
		LDFLAGS=$am_save_LDFLAGS
	else
		AC_MSG_RESULT(no)
	fi
	AC_SUBST(LIBCDDB_CFLAGS)
	AC_SUBST(LIBCDDB_LDFLAGS)

	if test $have_libcddb = "yes"
	then
  		AC_DEFINE(HAVE_LIBCDDB, 1, [Define if you have the libcddb library])
		ifelse([$1], , :, [$1])
	else
		ifelse([$2], , :, [$2])
	fi
	
])