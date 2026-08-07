dnl AX_PATH_LIB_PPLIB([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AX_PATH_LIB_PPLIB],[dnl

AC_ARG_WITH([libpplib],
	[  --with-libpplib[[=PATH]]  Prefix where PPLIB-Library is installed],
	[pplib_prefix="$withval"],
	[pplib_prefix="no"])

#if test "$pplib_prefix" != "no"
#then
	if test "$pplib_prefix" = "no"
	then
		AC_PATH_PROG(pplibconfig,pplib-config)
	elif test "$pplib_prefix" != "yes"
	then
		pplibconfig="$pplib_prefix/bin/pplib-config"
	else
		AC_PATH_PROG(pplibconfig,pplib-config)
	fi
	
	AC_MSG_CHECKING([for lib pplib])
	if test [ -z "$pplibconfig" ]
	then
		AC_MSG_RESULT(no)
	    AC_MSG_ERROR([pplib library (libpplib) and/or headers not found])
		
		ifelse([$3], , :, [$3])
	else
		AC_MSG_RESULT(yes)
		min_ppl_version=ifelse([$1], ,6.0.0,[$1])
		AC_MSG_CHECKING(for pplib version >= $min_ppl_version)
		
		ppl_version=`${pplibconfig} --version`
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
			#AC_MSG_CHECKING(pplib debug libraries)
			LIBPPLIB_DEBUG_LIBS=`${pplibconfig} --libs debug`
			#AC_MSG_RESULT($LIBPPLIB_DEBUG_LIBS)
			#AC_MSG_CHECKING(pplib release libraries)
			LIBPPLIB_RELEASE_LIBS=`${pplibconfig} --libs release`
			LIBPPLIB_RELEASE_ARCHIVE=`${pplibconfig} --archive release`
			LIBPPLIB_DEBUG_ARCHIVE=`${pplibconfig} --archive debug`
			#AC_MSG_RESULT($LIBPPLIB_RELEASE_LIBS)
			#AC_MSG_CHECKING(pplib includes)
			LIBPPLIB_CFLAGS=`${pplibconfig} --cflags`
			LIBPPLIB=`${pplibconfig} --ppllib release`
			LIBPPLIB_DEBUG=`${pplibconfig} --ppllib debug`
			
			#AC_MSG_RESULT($LIBPPLIB_CFLAGS)
			ifelse([$2], , :, [$2])
		fi
	fi
#else
#	AC_MSG_RESULT(not configured)
#	AC_MSG_ERROR([pplib library is required])
#fi
])



dnl AX_PPLIB_FEATURE([FEATURE, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN([AX_PPLIB_FEATURE],[dnl
	AC_MSG_CHECKING([for pplib-feature: $1])
	if test -z "${ppl_features}"
	then
		ppl_features=`${pplibconfig} --features`
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

