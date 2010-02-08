dnl AC_CHECK_PGSQL([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_PGSQL],[

AC_ARG_WITH([pgsql],
	[  --with-pgsql[[=PATH]]     Prefix where PostgreSQL is installed (optional)],
	[pgsql_prefix="$withval"],
	[pgsql_prefix=""])

if test "$pgsql_prefix" != "no"
then
	AC_PATH_PROG(pg_config,pg_config)
	if test [ -n "$pgsql_prefix" ]
	then
		if test	[ "$pgsql_prefix" != "yes" ]
		then
			pg_config="$pgsql_prefix/bin/pg_config"
		fi
	fi
	if test [ ! -x "$pg_config" ]
	then
		pg_config=""
	fi
	if test [ -z "$pg_config" ]
	then
		ifelse([$3], , :, [$3])
	else
		AC_MSG_CHECKING(pgsql libraries)
		PGSQL_LIBS="-L`${pg_config} --libdir` -lpq"
		AC_MSG_RESULT($PGSQL_LIBS)
		AC_MSG_CHECKING(pgsql includes)
		PGSQL_CFLAGS="-I`${pg_config} --includedir`"
		AC_MSG_RESULT($PGSQL_CFLAGS)
		ifelse([$2], , :, [$2])
	fi
fi
])

