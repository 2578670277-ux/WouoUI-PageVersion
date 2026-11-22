#ifndef __MACRO_H__
#define __MACRO_H__

//字符串宏
#define _STRCAT2(a,b) a##b
#define STRCAT2(a,b) _STRCAT2(a,b)
//拼接宏
#define _STR(x) #x
#define STR(x) _STR(x)


#endif
