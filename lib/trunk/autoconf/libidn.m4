dnl AM_LIBIDN([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AM_LIBIDN],[
AC_ARG_WITH([libidn],
	[  --with-libidn[[=PATH]]     Prefix where libidn is installed (optional)],
	[libidn_prefix="$withval"],
	[libidn_prefix="no"])

am_save_CFLAGS="$CFLAGS"
am_save_LIBS="$LIBS"
am_save_LDFLAGS="$LDFLAGS"
LIBIDN_LIBS="-lidn"
LIBIDN_CFLAGS=""
have_libidn="no"
	if test "$libidn_prefix" != "yes"
	then
		if test "$libidn_prefix" != "no"
		then
			LIBS="-L$libidn_prefix/lib -lidn $LIBS"
			CFLAGS="$CFLAGS -I$libidn_prefix/include"
			LIBIDN_LIBS="-L$libidn_prefix/lib -lidn"
			LIBIDN_CFLAGS="-I$libidn_prefix/include"
		else
			LIBS="-lidn $LIBS"
			LIBIDN_LIBS="-lidn"
		fi
	else
		LIBS="-lidn $LIBS"
		LIBIDN_LIBS="-lidn"
	fi
	AC_CHECK_HEADER([idna.h], [
	
		AC_MSG_CHECKING([for idn library and linking])
    	AC_LINK_IFELSE(
      	[[
        	 #include <idna.h>
         	int main()
         	{
         		char *idn=NULL;
         		char *a=NULL;
				if (IDNA_SUCCESS==idna_to_ascii_8z(idn, &a,0) && a!=NULL) {
					return 1;
				}
         		return 0;
         	}
      	]],
      	[AC_MSG_RESULT(yes)
      	have_libidn="yes"
        ],
    	[
         AC_MSG_RESULT(no)
      	]
    	)
    ])
CFLAGS=$am_save_CFLAGS
LIBS=$am_save_LIBS
LDFLAGS=$am_save_LDFLAGS

if test "$have_libidn" = "yes"
then
	ifelse([$2], , :, [$2])
else
	LIBIDN_LIBS=""
	LIBIDN_CFLAGS=""
	ifelse([$3], , :, [$3])
fi


])
