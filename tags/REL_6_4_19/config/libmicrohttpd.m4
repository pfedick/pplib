dnl AM_LIBMICROHTTPD([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AM_LIBMICROHTTPD],[dnl
AC_ARG_WITH([libmicrohttpd],
	[  --with-libmicrohttpd[[=PATH]]     Prefix where libmicrohttpd is installed (optional)],
	[libmicrohttpd_prefix="$withval"],
	[libmicrohttpd_prefix="no"])


AC_MSG_CHECKING([for if libmicrohttpd should be used])
if test "$with_libmicrohttpd" != "no"
then
	AC_MSG_RESULT(yes)
	am_save_CPPFLAGS="$CPPFLAGS"
	am_save_LIBS="$LIBS"
	am_save_LDFLAGS="$LDFLAGS"
	LIBS="-lmicrohttpd $PTHREAD_LIBS $LIBGCRYPT_LIBS $LIBGNUTLS_LIBS $LIBS"
	CPPFLAGS="$CPPFLAGS $PTHREAD_CFLAGS"
	LIBMICROHTTPD_LIBS="-lmicrohttpd"
	LIBMICROHTTPD_CFLAGS=""
	
	if test "$libmicrohttpd_prefix" != "yes"
	then
		LIBS="-L$libmicrohttpd_prefix/lib -lmicrohttpd $PTHREAD_LIBS $LIBGCRYPT_LIBS $LIBGNUTLS_LIBS $LIBS"
		CPPFLAGS="$CPPFLAGS -I$libmicrohttpd_prefix/include $PTHREAD_CFLAGS $LIBGCRYPT_CFLAGS $LIBGNUTLS_CFLAGS"
		LIBMICROHTTPD_LIBS="-L$libmicrohttpd_prefix/lib -lmicrohttpd"
		LIBMICROHTTPD_CFLAGS="-I$libmicrohttpd_prefix/include"
	fi
	AC_CHECK_LIB(rt,clock_gettime,
			LIBMICROHTTPD_LIBS="$LIBMICROHTTPD_LIBS -lrt "
			LIBS="$LIBS -lrt"
		)
	AC_MSG_CHECKING([if we can link against libmicrohttpd])
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
      	dnl Vielleicht ist gnutls erforderlich
		LIBMICROHTTPD_LIBS="$LIBMICROHTTPD_LIBS -lgnutls "
		LIBS="$LIBS -lgnutls"
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
        	LIBMICROHTTPD_LIBS=""
        	LIBMICROHTTPD_CFLAGS=""        	
        	ifelse([$3], , :, [$3])
        ])
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
