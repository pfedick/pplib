dnl AC_CHECK_NASM([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_CHECK_NASM],[

AC_ARG_WITH([nasm],
	[  --with-nasm[[=PATH]]      Prefix where nasm is installed (optional)],
	[nasm_prefix="$withval"],
	[nasm_prefix="no"])

if test "$nasm_prefix" != "no"
then
	if test "$nasm_prefix" != "yes"
	then
		nasm="$nasm_prefix/bin/nasm"
	else
		AC_PATH_PROG(nasm,nasm)
	fi
	if test [ -n "$nasm" ]
	then
		NASM=${nasm}
		AC_SUBST(NASM)
		case `uname -m` in
			win32)
				NASMFLAGS="-f win32"
				NASMDEBUGFLAGS="-g"
				;;
			win64)
				NASMFLAGS="-f win64"
				NASMDEBUGFLAGS="-g"
				;;
			x86_64|amd64)
				NASMFLAGS="-f elf64"
				NASMDEBUGFLAGS="-g"
				;;
			*)
				NASMFLAGS="-f elf32"
				NASMDEBUGFLAGS="-g"
				;;
		esac
		case `uname -s` in
			MINGW32*)
				NASMFLAGS="-f win32"
				NASMDEBUGFLAGS="-g"
				;;
			
		esac
		
		AC_SUBST(NASMFLAGS)
		AC_SUBST(NASMDEBUGFLAGS)
		ifelse([$2], , :, [$2])
	fi
fi
])

