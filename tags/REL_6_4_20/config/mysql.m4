dnl AC_CHECK_MYSQL([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_MYSQL],[
AC_MSG_CHECKING(for MySQL)

AC_ARG_WITH([mysql],
	[  --with-mysql[[=PATH]]     Prefix where mysql is installed (optional)],
	[mysql_prefix="$withval"],
	[mysql_prefix="no"])

if test "$mysql_prefix" != "no"
then

	if test "$mysql_prefix" != "yes"
	then
		mysqlconfig="$mysql_prefix/bin/mysql_config"
	else
		AC_PATH_PROG(mysqlconfig,mysql_config)
	fi
	if test [ -z "$mysqlconfig" ]
	then
		ifelse([$3], , :, [$3])
	else
		AC_MSG_CHECKING(mysql libraries)
		MYSQL_LIBS=`${mysqlconfig} --libs_r`
		AC_MSG_RESULT($MYSQL_LIBS)
		AC_MSG_CHECKING(mysql includes)
		MYSQL_CFLAGS=`${mysqlconfig} --include`
		AC_MSG_RESULT($MYSQL_CFLAGS)
		ifelse([$2], , :, [$2])
	fi
fi
])

