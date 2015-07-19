dnl AC_CHECK_YASM([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_YASM],[

AC_ARG_WITH([yasm],
	[  --with-yasm[[=PATH]]      Prefix where yasm is installed (optional)],
	[yasm_prefix="$withval"],
	[yasm_prefix="no"])

if test "$yasm_prefix" != "no"
then
	if test "$yasm_prefix" != "yes"
	then
		yasm="$yasm_prefix/bin/yasm"
	else
		AC_PATH_PROG(yasm,yasm)
	fi
	if test [ -n "$yasm" ]
	then
		YASM=${yasm}
		AC_SUBST(YASM)
		case `uname -m` in
			win32)
				YASMFLAGS="-f win32 -m x86"
				YASMDEBUGFLAGS="-g"
				;;
			win64)
				YASMFLAGS="-f win64 -m amd64"
				YASMDEBUGFLAGS="-g"
				;;
			x86_64)
				YASMFLAGS="-f elf64 -m amd64"
				YASMDEBUGFLAGS="-g"
				;;
			*)
				YASMFLAGS="-f elf32 -m x86"
				YASMDEBUGFLAGS="-g"
				;;
		esac
		case `uname -s` in
			MINGW32*)
				YASMFLAGS="-f win32 -m x86"
				YASMDEBUGFLAGS="-g"
				;;
			
		esac
		AC_SUBST(YASMFLAGS)
		AC_SUBST(YASMDEBUGFLAGS)
		ifelse([$2], , :, [$2])
	fi
fi
])

