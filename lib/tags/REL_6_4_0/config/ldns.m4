dnl AX_PATH_LIB_LDNS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AX_PATH_LIB_LDNS],[dnl

AC_ARG_WITH([libldns],
	[  --with-libldns[[=PATH]]  Prefix where LDNS-Library from netlabs.nl is installed],
	[ldns_prefix="$withval"],
	[ldns_prefix="no"])

	if test "$ldns_prefix" = "no"
	then
		AC_PATH_PROG(ldnsconfig,ldns-config)
	elif test "$ldns_prefix" != "yes"
	then
		ldnsconfig="$ldns_prefix/bin/ldns-config"
	else
		AC_PATH_PROG(ldnsconfig,ldns-config)
	fi
	
	AC_MSG_CHECKING([for lib ldns])
	if test [ -z "$ldnsconfig" ]
	then
		AC_MSG_RESULT(no)
		
		ifelse([$3], , :, [$3])
	else
		AC_MSG_RESULT(yes)
		min_ldns_version=ifelse([$1], ,1.6.0,[$1])
		AC_MSG_CHECKING(for ldns version >= $min_ldns_version)
		if test [ -f ${ldnsconfig} ]
		then
			ldns_version=`${ldnsconfig} --version`
		elif test [ -f $ldns_prefix/bin/ldns-version ]
		then
			ldns_version=`$ldns_prefix/bin/ldns-version`
		fi
			
		ldns_config_major_version=`echo $ldns_version | \
			sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    	ldns_config_minor_version=`echo $ldns_version | \
			sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
		ldns_config_micro_version=`echo $ldns_version | \
			sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
		ldns_config_version=`expr $ldns_config_major_version \* 10000 + $ldns_config_minor_version \* 100 + $ldns_config_micro_version`

		ldns_req_major_version=`echo $min_ldns_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    	ldns_req_minor_version=`echo $min_ldns_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
		ldns_req_micro_version=`echo $min_ldns_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
		ldns_req_version=`expr $ldns_req_major_version \* 10000 + $ldns_req_minor_version \* 100 + $ldns_req_micro_version`
		
		if test $ldns_config_version -lt $ldns_req_version
		then
			AC_MSG_RESULT([no, have $ldns_version])
			ifelse([$3], , :, [$3])
		else 
			AC_MSG_RESULT([yes (version $ldns_version) ])
			LIBLDNS_LIBS=`${ldnsconfig} --libs`
			LIBLDNS_CFLAGS=`${ldnsconfig} --cflags`
			ifelse([$2], , :, [$2])
		fi
	fi
])




