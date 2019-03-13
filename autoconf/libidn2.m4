dnl AM_LIBIDN([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AM_LIBIDN2],[
AC_ARG_WITH([libidn2],
	[  --with-libidn2[[=PATH]]     Prefix where libidn2 is installed (optional)],
	[libidn2_prefix="$withval"],
	[libidn2_prefix="no"])

am_save_CPPFLAGS="$CPPFLAGS"
am_save_CFLAGS="$CFLAGS"
am_save_LIBS="$LIBS"
am_save_LDFLAGS="$LDFLAGS"
LIBIDN2_LIBS=""
LIBIDN2_CFLAGS=""
have_libidn="no"
	if test "$libidn2_prefix" != "yes"
	then
		if test "$libidn2_prefix" != "no"
		then
			LIBS="-L$libidn2_prefix/lib -lidn2 $LIBS"
			CPPFLAGS="$CPPFLAGS -I$libidn2_prefix/include"
			CFLAGS="$CFLAGS -I$libidn2_prefix/include"
			LIBIDN2_LIBS="-L$libidn2_prefix/lib -lidn2"
			LIBIDN2_CFLAGS="-I$libidn2_prefix/include"
		else
			LIBS="-lidn2 $LIBS"
			LIBIDN2_LIBS="-lidn2"
		fi
	else
		LIBS="-lidn2 $LIBS"
		LIBIDN2_LIBS="-lidn2"
	fi
	AC_CHECK_HEADER([idn2.h], [
	
		AC_MSG_CHECKING([for idn2 library and linking])
    	AC_LINK_IFELSE([AC_LANG_SOURCE(
      	[[
        	 #include <idn2.h>
         	int main()
         	{
         		char *idn=NULL;
         		char *a=NULL;
				if (IDNA_SUCCESS==idn2_to_ascii_8z(idn, &a,0) && a!=NULL) {
					return 1;
				}
         		return 0;
         	}
      	]])],
      	[AC_MSG_RESULT(yes)
      	have_libidn2="yes"
        ],
    	[
         AC_MSG_RESULT(no)
      	]
    	)
    	AC_MSG_CHECKING([for existing IDN2_NO_TR46-flag in idn2 library])
    	AC_LINK_IFELSE([AC_LANG_SOURCE(
      	[[
        	 #include <idn2.h>
         	int main()
         	{
         		char *idn=NULL;
         		char *a=NULL;
				if (IDNA_SUCCESS==idn2_to_ascii_8z(idn, &a,IDN2_NO_TR46) && a!=NULL) {
					return 1;
				}
         		return 0;
         	}
      	]])],
      	[AC_MSG_RESULT(yes)
      	have_libidn2_IDN2_NO_TR46="yes"
      	AC_DEFINE(HAVE_LIBIDN2_IDN2_NO_TR46, 1, [ Define if libidn2 has IDN2_NO_TR46 flag. ])
        ],
    	[
         AC_MSG_RESULT(no)
      	]
    	)
    ])
CFLAGS=$am_save_CFLAGS    
CPPFLAGS=$am_save_CPPFLAGS
LIBS=$am_save_LIBS
LDFLAGS=$am_save_LDFLAGS

if test "$have_libidn2" = "yes"
then
	ifelse([$2], , :, [$2])
else
	ifelse([$3], , :, [$3])
fi


])
