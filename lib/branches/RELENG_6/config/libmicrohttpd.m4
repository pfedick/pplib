dnl AM_LIBMICROHTTPD([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AM_LIBMICROHTTPD],[dnl
AC_MSG_CHECKING([for libmicrohttpd library and headers])
AC_ARG_WITH([libmicrohttpd],
	[  --with-libmicrohttpd[[=PATH]]     Prefix where libmicrohttpd is installed (optional)],
	[libmicrohttpd_prefix="$withval"],
	[libmicrohttpd_prefix="no"])


if test "$libmicrohttpd_prefix" != "no"
then
	am_save_CPPFLAGS="$CPPFLAGS"
	am_save_LIBS="$LIBS"
	am_save_LDFLAGS="$LDFLAGS"
	LIBS="-lmicrohttpd -pthread $LIBGCRYPT_LIBS $LIBGNUTLS_LIBS $LIBS"
	LIBMICROHTTPD_LIBS="-lmicrohttpd"
	LIBMICROHTTPD_CFLAGS=""
	
	if test "$libmicrohttpd_prefix" != "yes"
	then
		LIBS="-L$libmicrohttpd_prefix/lib -lmicrohttpd -pthread $LIBGCRYPT_LIBS $LIBGNUTLS_LIBS $LIBS"
		CPPFLAGS="$CPPFLAGS -I$libmicrohttpd_prefix/include $LIBGCRYPT_CFLAGS $LIBGNUTLS_CFLAGS"
		LIBMICROHTTPD_LIBS="-L$libmicrohttpd_prefix/lib -lmicrohttpd"
		LIBMICROHTTPD_CFLAGS="-I$libmicrohttpd_prefix/include"
	fi
    AC_LINK_IFELSE( [AC_LANG_SOURCE([[
         #include <microhttpd.h>
         int main()
         {
         	struct MHD_Daemon* daemon;
         	MHD_stop_daemon(daemon);
         }
      ]]) ],
      [AC_MSG_RESULT(yes)
      ifelse([$2], , :, [$2])
    
      ],
      [
      	AC_MSG_RESULT(no)
        ifelse([$3], , :, [$3])
      ]
    )

	CPPFLAGS=$am_save_CPPFLAGS
	LIBS=$am_save_LIBS
	LDFLAGS=$am_save_LDFLAGS
else
	AC_MSG_RESULT(no)
	ifelse([$3], , :, [$3])
fi


])
