dnl AM_PATH_LIBMHASH([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AM_PATH_LIBMHASH],[dnl
AC_MSG_CHECKING([for libmhash])
AC_ARG_WITH([libmhash],
	[  --with-libmhash[[=PATH]]     Prefix where libmhash is installed (optional)],
	[libmhash_prefix="$withval"],
	[libmhash_prefix="no"])

am_save_CPPFLAGS="$CPPFLAGS"
am_save_LIBS="$LIBS"
am_save_LDFLAGS="$LDFLAGS"
LIBMHASH_LIBS="-lmhash"
LIBMHASH_CFLAGS=""
have_libmhash="no"
	if test "$libmhash_prefix" != "yes"
	then
		if test "$libmhash_prefix" != "no"
		then
			LIBS="-L$libmhash_prefix/lib -lmhash"
			CPPFLAGS="-I$libmhash_prefix/include"
			LIBMHASH_LIBS="-L$libmhash_prefix/lib -lmhash"
			LIBMHASH_CFLAGS="-I$libmhash_prefix/include"
		fi
	else
		LIBS="$LIBS -lmhash"
		LIBMHASH_LIBS="-lmhash"
	fi
	if test "$libmhash_prefix" != "no"
	then 
    AC_LINK_IFELSE(
      [[
         #include <mhash.h>
         int main()
         {
            void *handle=mhash_init(MHASH_MD5);
         }
      ]],
      [AC_MSG_RESULT(yes)
      	have_libmhash="yes"
      	AC_SUBST(LIBMHASH_LIBS)
      	AC_SUBST(LIBMHASH_CFLAGS)
    
      ],
      [
         AC_MSG_RESULT(no)
      ]
    )
    fi

CPPFLAGS=$am_save_CPPFLAGS
LIBS=$am_save_LIBS
LDFLAGS=$am_save_LDFLAGS

if test "$have_libmhash" = "yes"
then
	ifelse([$2], , :, [$2])
else
	LIBMHASH_LIBS=""
	LIBMHASH_CFLAGS=""
	ifelse([$3], , :, [$3])
fi


])
