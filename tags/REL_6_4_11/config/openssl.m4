AC_DEFUN([AC_FIND_OPENSSL], [
  incs="$1"
  libs="$2"
  case "$incs---$libs" in
    ---)
      for d in /usr/ssl/include /usr/local/ssl/include /usr/include \
/usr/include/ssl /opt/ssl/include /opt/openssl/include \
/usr/local/ssl/include /usr/local/include /usr/freeware/include ; do
       if test -f $d/openssl/ssl.h  ; then
         OPENSSL_INCLUDE=-I$d
       fi
      done

      for d in /usr/ssl/lib /usr/local/ssl/lib /usr/lib/openssl \
/usr/lib /usr/lib64 /opt/ssl/lib /opt/openssl/lib \
/usr/freeware/lib32 /usr/local/lib/ ; do
      # Just to be safe, we test for ".so" anyway
      if test -f $d/libssl.a || test -f $d/libssl.so || test -f $d/libssl$shrext_cmds ; then
        OPENSSL_LIB=$d
      fi
      done
      ;;
    ---* | *---)
      AC_MSG_ERROR([if either 'includes' or 'libs' is specified, both must be specified])
      ;;
    * )
      if test -f $incs/openssl/ssl.h  ; then
        OPENSSL_INCLUDE=-I$incs
      fi
      # Just to be safe, we test for ".so" anyway
      if test -f $libs/libssl.a || test -f $libs/libssl.so || test -f $libs/libssl$shrext_cmds ; then
        OPENSSL_LIB=$libs
      fi
      ;;
  esac

  # On RedHat 9 we need kerberos to compile openssl
  for d in /usr/kerberos/include
  do
   if test -f $d/krb5.h  ; then
     OPENSSL_KERBEROS_INCLUDE="$d"
   fi
  done


 if test -z "$OPENSSL_LIB" -o -z "$OPENSSL_INCLUDE" ; then
   echo "Could not find an installation of OpenSSL"
   if test -n "$OPENSSL_LIB" ; then
    if test "$TARGET_LINUX" = "true"; then
      echo "Looks like you've forgotten to install OpenSSL development RPM"
    fi
   fi
  exit 1
 fi

])

AC_DEFUN([AC_CHECK_OPENSSL], [
AC_MSG_CHECKING(for OpenSSL)
  AC_ARG_WITH([openssl],
              [  --with-openssl[=DIR]      Include the OpenSSL support],
              [openssl="$withval"],
              [openssl=no])

  AC_ARG_WITH([openssl-includes],
              [
  --with-openssl-includes=DIR
                          Find OpenSSL headers in DIR],
              [OPENSSL_INCLUDES="$withval"],
              [OPENSSL_INCLUDES=""])

  AC_ARG_WITH([openssl-libs],
              [
  --with-openssl-libs=DIR
                          Find OpenSSL libraries in DIR],
              [OPENSSL_LIBS="$withval"],
              [OPENSSL_LIBS=""])

  if test "$openssl" != "no"
  then
	if test "$openssl" != "yes"
	then
		if test -z "$OPENSSL_INCLUDES" 
		then
			OPENSSL_INCLUDES="$openssl/include"
		fi
		if test -z "$OPENSSL_LIBS" 
		then
			OPENSSL_LIBS="$openssl/lib"
		fi
	fi
    AC_FIND_OPENSSL([$OPENSSL_INCLUDES], [$OPENSSL_LIBS])
    #force VIO use
    AC_MSG_RESULT(yes)
    OPENSSL_LIBS="-L$OPENSSL_LIB -lssl -lcrypto"
    case "${host_cpu}-${host_os}" in
        *solaris*)
		OPENSSL_LIBS="$OPENSSL_LIBS -ldl"
        ;;
	esac
    
    # Don't set OPENSSL_INCLUDES to /usr/include as this gives us a lot of
    # compiler warnings when using gcc 3.x
    OPENSSL_INCLUDES=""
    if test "$OPENSSL_INCLUDE" != "-I/usr/include"
    then
	OPENSSL_INCLUDES="$OPENSSL_INCLUDE"
    fi
    if test "$OPENSSL_KERBEROS_INCLUDE"
    then
    	OPENSSL_INCLUDES="$OPENSSL_INCLUDES -I$OPENSSL_KERBEROS_INCLUDE"
    fi
    AC_DEFINE([HAVE_OPENSSL], [1], [OpenSSL])
	report_have_openssl="yes"

    # openssl-devel-0.9.6 requires dlopen() and we can't link staticly
    # on many platforms (We should actually test this here, but it's quite
    # hard) to do as we are doing libtool for linking.
    using_static=""
    case "$CLIENT_EXTRA_LDFLAGS " in
	*-all-static*) using_static="yes" ;;
    esac
    if test "$using_static" = "yes"
    then
      echo "You can't use the --all-static link option when using openssl."
      exit 1
    fi
    NON_THREADED_CLIENT_LIBS="$NON_THREADED_CLIENT_LIBS $OPENSSL_LIBS"
  else
    AC_MSG_RESULT(no)
	if test ! -z "$OPENSSL_INCLUDES"
	then
		AC_MSG_ERROR(Can't have --with-openssl-includes without --with-openssl);
	fi
	if test ! -z "$OPENSSL_LIBS"
	then
		AC_MSG_ERROR(Can't have --with-openssl-libs without --with-openssl);
	fi
  fi
  AC_SUBST(OPENSSL_LIBS)
  AC_SUBST(OPENSSL_INCLUDES)
])