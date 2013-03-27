dnl AM_CHECK_LIBBIND()
AC_DEFUN([AM_CHECK_LIBBIND],[
AC_ARG_WITH([libbind],
	[  --with-libbind[[=PATH]]     Prefix where libbind is installed (optional)],
	[libbind_prefix="$withval"],
	[libbind_prefix="no"])


dnl res_query
AC_CHECK_LIB(resolv, res_query,,AC_CHECK_LIB(bind, res_query,,
	[dnl Have to include resolv.h as res_query is sometimes defined as a macro
	AC_MSG_CHECKING([for res_query in -lresolv (with resolv.h if present)])
	saved_libs="$LIBS"
	LIBS="-lresolv $LIBS"
	AC_TRY_LINK([
		#ifdef HAVE_RESOLV_H
		#include <resolv.h>
		#endif],
		[res_query(0,0,0,0,0)],
	[AC_MSG_RESULT(yes)
		have_res_query=yes],
	[AC_MSG_RESULT(no)
		LIBS="$saved_libs"])
	]
))

if test "$have_res_query" = "yes"
then
	AC_DEFINE(HAVE_RES_QUERY,1, [Define if you have res_query function])
fi


dnl res_search
AC_CHECK_LIB(resolv, res_search,,AC_CHECK_LIB(bind, res_search,,
	[dnl Have to include resolv.h as res_query is sometimes defined as a macro
	AC_MSG_CHECKING([for res_search in -lresolv (with resolv.h if present)])
	saved_libs="$LIBS"
	LIBS="-lresolv $LIBS"
	AC_TRY_LINK([
		#ifdef HAVE_RESOLV_H
		#include <resolv.h>
		#endif],
		[res_search(0,0,0,0,0)],
	[AC_MSG_RESULT(yes)
		have_res_search=yes],
	[AC_MSG_RESULT(no)
		LIBS="$saved_libs"])
	]
))

if test "$have_res_search" = "yes"
then
	AC_DEFINE(HAVE_RES_SEARCH,1, [Define if you have res_search function])
fi

dnl res_querydomain
AC_CHECK_LIB(resolv, res_querydomain,,AC_CHECK_LIB(bind, res_querydomain,,
	[dnl Have to include resolv.h as res_query is sometimes defined as a macro
	AC_MSG_CHECKING([for res_querydomain in -lresolv (with resolv.h if present)])
	saved_libs="$LIBS"
	LIBS="-lresolv $LIBS"
	AC_TRY_LINK([
		#ifdef HAVE_RESOLV_H
		#include <resolv.h>
		#endif],
		[res_querydomain(0,0,0,0,0,0)],
	[AC_MSG_RESULT(yes)
		have_res_querydomain=yes],
	[AC_MSG_RESULT(no)
		LIBS="$saved_libs"])
	]
))

if test "$have_res_querydomain" = "yes"
then
	AC_DEFINE(HAVE_RES_QUERYDOMAIN,1, [Define if you have res_querydomain function])
fi

dnl res_mkquery
AC_CHECK_LIB(resolv, res_mkquery,,AC_CHECK_LIB(bind, res_mkquery,,
	[dnl Have to include resolv.h as res_query is sometimes defined as a macro
	AC_MSG_CHECKING([for res_mkquery in -lresolv (with resolv.h if present)])
	saved_libs="$LIBS"
	LIBS="-lresolv $LIBS"
	AC_TRY_LINK([
		#ifdef HAVE_RESOLV_H
		#include <resolv.h>
		#endif],
		[res_mkquery(0,0,0,0,0,0,0,0,0)],
	[AC_MSG_RESULT(yes)
		have_res_mkquery=yes],
	[AC_MSG_RESULT(no)
		LIBS="$saved_libs"])
	]
))

if test "$have_res_mkquery" = "yes"
then
	AC_DEFINE(HAVE_RES_MKQUERY,1, [Define if you have res_mkquery function])
fi

dnl res_send
AC_CHECK_LIB(resolv, res_send,,AC_CHECK_LIB(bind, res_send,,
	[dnl Have to include resolv.h as res_query is sometimes defined as a macro
	AC_MSG_CHECKING([for res_send in -lresolv (with resolv.h if present)])
	saved_libs="$LIBS"
	LIBS="-lresolv $LIBS"
	AC_TRY_LINK([
		#ifdef HAVE_RESOLV_H
		#include <resolv.h>
		#endif],
		[res_send(0,0,0,0)],
	[AC_MSG_RESULT(yes)
		have_res_send=yes],
	[AC_MSG_RESULT(no)
		LIBS="$saved_libs"])
	]
))

if test "$have_res_send" = "yes"
then
	AC_DEFINE(HAVE_RES_SEND,1, [Define if you have res_send function])
fi

dnl dn_comp
AC_CHECK_LIB(resolv, dn_comp,,AC_CHECK_LIB(bind, dn_comp,,
	[dnl Have to include resolv.h as res_query is sometimes defined as a macro
	AC_MSG_CHECKING([for dn_comp in -lresolv (with resolv.h if present)])
	saved_libs="$LIBS"
	LIBS="-lresolv $LIBS"
	AC_TRY_LINK([
		#ifdef HAVE_RESOLV_H
		#include <resolv.h>
		#endif],
		[dn_comp(0,0,0,0,0)],
	[AC_MSG_RESULT(yes)
		have_dn_comp=yes],
	[AC_MSG_RESULT(no)
		LIBS="$saved_libs"])
	]
))

if test "$have_dn_comp" = "yes"
then
	AC_DEFINE(HAVE_DN_COMP,1, [Define if you have dn_comp function])
fi

dnl dn_expand
AC_CHECK_LIB(resolv, dn_expand,,AC_CHECK_LIB(bind, dn_expand,,
	[dnl Have to include resolv.h as dn_expand is sometimes defined as a macro
	AC_MSG_CHECKING([for dn_expand in -lresolv (with resolv.h if present)])
	saved_libs="$LIBS"
	LIBS="-lresolv $LIBS"
	AC_TRY_LINK([
		#ifdef HAVE_RESOLV_H
		#include <resolv.h>
		#endif],
		[dn_expand(0,0,0,0,0)],
	[AC_MSG_RESULT(yes)
		have_dn_expand=yes],
	[AC_MSG_RESULT(no)
		LIBS="$saved_libs"])
	]
))

if test "$have_dn_expand" = "yes"
then
	AC_DEFINE(HAVE_DN_EXPAND,1, [Define if you have dn_expand function])
fi

])
