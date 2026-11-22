#ifndef __WARNINGMASK_H__
#define __WARNINGMASK_H__

//用于屏蔽警告(包含于对应的.c即可)
//..\LetterShell\shell_companion.c: 1 warning, 0 errors
//..\LetterShell\Log\log.c(137): warning:  #188-D: enumerated type mixed with another type
//..\HAREWARE\PACK\pack.c(79): warning:  #1256-D: "char" would have been promoted to "int" when passed through the ellipsis parameter; use the latter type instead temp_char = a_arg(arg, char);

#pragma diag_suppress 174
#pragma diag_suppress 188
#pragma diag_suppress 1256


#endif

