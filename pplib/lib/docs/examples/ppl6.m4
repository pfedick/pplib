dnl AX_PATH_LIB_PPL6([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AX_PATH_LIB_PPL6],[dnl

AC_ARG_WITH([libppl6],
	[  --with-libppl6[[=PATH]]  Prefix where PPL6-Library is installed],
	[ppl6_prefix="$withval"],
	[ppl6_prefix="no"])

#if test "$ppl6_prefix" != "no"
#then
	if test "$ppl6_prefix" = "no"
	then
		AC_PATH_PROG(ppl6config,ppl6-config)
	elif test "$ppl6_prefix" != "yes"
	then
		ppl6config="$ppl6_prefix/bin/ppl6-config"
	else
		AC_PATH_PROG(ppl6config,ppl6-config)
	fi
	
	AC_MSG_CHECKING([for lib ppl6])
	if test [ -z "$ppl6config" ]
	then
		AC_MSG_RESULT(no)
	    AC_MSG_ERROR([ppl6 library (libppl6) and/or headers not found])
		
		ifelse([$3], , :, [$3])
	else
		AC_MSG_RESULT(yes)
		min_ppl_version=ifelse([$1], ,6.0.0,[$1])
		AC_MSG_CHECKING(for ppl6 version >= $min_ppl_version)
		
		ppl_version=`${ppl6config} --version`
		ppl_config_major_version=`echo $ppl_version | \
			sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    	ppl_config_minor_version=`echo $ppl_version | \
			sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
		ppl_config_micro_version=`echo $ppl_version | \
			sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
		ppl_config_version=`expr $ppl_config_major_version \* 10000 + $ppl_config_minor_version \* 100 + $ppl_config_micro_version`

		ppl_req_major_version=`echo $min_ppl_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    	ppl_req_minor_version=`echo $min_ppl_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
		ppl_req_micro_version=`echo $min_ppl_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
		ppl_req_version=`expr $ppl_req_major_version \* 10000 + $ppl_req_minor_version \* 100 + $ppl_req_micro_version`
		
		if test $ppl_config_version -lt $ppl_req_version
		then
			AC_MSG_RESULT([no, have $ppl_version])
			ifelse([$3], , :, [$3])
		else 
			AC_MSG_RESULT([yes (version $ppl_version) ])
			#AC_MSG_CHECKING(ppl6 debug libraries)
			LIBPPL6_DEBUG_LIBS=`${ppl6config} --libs debug`
			#AC_MSG_RESULT($LIBPPL6_DEBUG_LIBS)
			#AC_MSG_CHECKING(ppl6 release libraries)
			LIBPPL6_RELEASE_LIBS=`${ppl6config} --libs release`
			LIBPPL6_RELEASE_ARCHIVE=`${ppl6config} --archive release`
			LIBPPL6_DEBUG_ARCHIVE=`${ppl6config} --archive debug`
			#AC_MSG_RESULT($LIBPPL6_RELEASE_LIBS)
			#AC_MSG_CHECKING(ppl6 includes)
			LIBPPL6_CFLAGS=`${ppl6config} --cflags`
			LIBPPL6=`${ppl6config} --ppllib release`
			LIBPPL6_DEBUG=`${ppl6config} --ppllib debug`
			
			#AC_MSG_RESULT($LIBPPL6_CFLAGS)
			ifelse([$2], , :, [$2])
		fi
	fi
#else
#	AC_MSG_RESULT(not configured)
#	AC_MSG_ERROR([ppl6 library is required])
#fi
])



dnl AX_PPL6_FEATURE([FEATURE, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AX_PPL6_FEATURE],[dnl
	AC_MSG_CHECKING([for ppl6-feature: $1])
	if test -z "${ppl_features}"
	then
		ppl_features=`${ppl6config} --features`
	fi
	echo ${ppl_features}| tr " " "\n" | grep -i "^$1" > /dev/null 2>&1
	if test $? -eq 0
	then
		AC_MSG_RESULT(yes)
		ifelse([$2], , :, [$2])
	else
		AC_MSG_RESULT(no)
		ifelse([$3], , :, [$3])
	fi
])

