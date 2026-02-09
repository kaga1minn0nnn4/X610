/*------------------------------------------------------------------------*/
/* Universal string handler for user console interface  (C)ChaN, 2011     */
/*------------------------------------------------------------------------*/

#ifndef _STRFUNC
#define _STRFUNC

#ifdef __cplusplus
extern "C" {
#endif

// xprintf global config
#define _USE_XFUNC_OUT	1	/* 1: Use output functions */
#define	_CR_CRLF		0	/* 1: Convert \n ==> \r\n in the output char */

#define _USE_XFUNC_IN	1	/* 1: Use input function */
#define	_LINE_ECHO		0	/* 1: Echo back input chars in xgets function */

#define _GET_LENGTH		1	/* 1: xsprintf return str length 0: no return (void) */
#define _USE_VA_LIST	1	/* 1: Use va_list in output functions */
#define _BUFFER_SIZE	32	/* Set xprintf internal buffer size (byte). minimum:16? */
#define _PROTECT_BUFFER	0	/* 1: Protect buffer 0: Not protect buffer!!! (fast) */

// xprintf feature select
#define _USE_ALL_FLAG	1	/* 1: Use "+", " ", and "#" */
#define _USE_PRECISION	1	/* 1: Use precision field */
#define _USE_ALL_LENGTH	1	/* 1: Use "h", "hh", "ll", and "L" */
#define _USE_ALL_INT	1	/* 1: Use "%i", "%n", and "%p" */
#define _USE_FLOATING	1	/* 1: Use "%e", "%f", "%g", and "%a" */
#define _USE_FAST_LONG	1	/* 1: Fast calc "long" */
#define _USE_FAST_LL	1	/* 1: Fast calc "long long" */
#define _USE_FAST_FLOAT	1	/* 1: Fast calc floating point */
#define _USE_ROUND_UP	1	/* 1: Floating round up */
#define _FLOATING_TYPE	1	/* 0: Use double 1:Use float (long double is not supported) */

// xprintf unuse function select
#define _DISABLE_SPACE	0	/* 1: Disable " " */
#define _DISABLE_PLUS	0	/* 1: Disable "+" (sign) */
#define _DISABLE_SHARP	0	/* 1: Disable "#" (prefix) */
#define _DISABLE_H		0	/* 1: Disable "h","hh" (short,char) */
#define _DISABLE_LL		0	/* 1: Disable "ll" (long long) */
#define _DISABLE_LONG_D	0	/* 1: Disable "L" (long double) */
#define _DISABLE_I		0	/* 1: Disable "%i" */
#define _DISABLE_C		0	/* 1: Disable "%c" */
#define _DISABLE_S		0	/* 1: Disable "%s" */
#define _DISABLE_O		0	/* 1: Disable "%o" */
#define _DISABLE_B		0	/* 1: Disable "%b" */
#define _DISABLE_X		0	/* 1: Disable "%x" */
#define _DISABLE_N		0	/* 1: Disable "%n" */
#define _DISABLE_P		0	/* 1: Disable "%p" */
#define _DISABLE_E		0	/* 1: Disable "%e" */
#define _DISABLE_F		0	/* 1: Disable "%f" */
#define _DISABLE_G		0	/* 1: Disable "%g" */
#define _DISABLE_A		0	/* 1: Disable "%a" */

#if _USE_XFUNC_OUT
#define xdev_out(func) xfunc_out = (void(*)(unsigned char))(func)
extern void (*xfunc_out)(unsigned char);
void xputc (char c);
void xputs (const char* str);
void xfputs (void (*func)(unsigned char), const char* str);

#if _USE_VA_LIST
#include <stdarg.h>
void xvprintf (const char* fmt, va_list arp);
#if _GET_LENGTH
int
#else
void
#endif
xvsprintf (char* buff, const char* fmt, va_list arp);
void xvfprintf (void (*func)(unsigned char), const char* fmt, va_list arp);
#endif

void xprintf (const char* fmt, ...);
#if _GET_LENGTH
int
#else
void
#endif
xsprintf (char* buff, const char* fmt, ...);
void xfprintf (void (*func)(unsigned char), const char* fmt, ...);
void put_dump (const void* buff, unsigned long addr, int len, int width);
#define DW_CHAR		sizeof(char)
#define DW_SHORT	sizeof(short)
#define DW_LONG		sizeof(long)
#endif

#if _USE_XFUNC_IN
#define xdev_in(func) xfunc_in = (unsigned char(*)(void))(func)
extern unsigned char (*xfunc_in)(void);
int xgets (char* buff, int len);
int xfgets (unsigned char (*func)(void), char* buff, int len);
int xatoi (char** str, long* res);
#endif

#ifdef __cplusplus
}
#endif

#endif
