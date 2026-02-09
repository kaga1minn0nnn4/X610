/*------------------------------------------------------------------------/
/  Universal string handler for user console interface
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2011, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#include "xprintf.h"


#if _USE_XFUNC_OUT
#include <stdarg.h>
void (*xfunc_out)(unsigned char);	/* Pointer to the output stream */
static char *outptr;

/*----------------------------------------------*/
/* Put a character                              */
/*----------------------------------------------*/

void xputc (char c)
{
	if (_CR_CRLF && c == '\n') xputc('\r');		/* CR -> CRLF */

	if (outptr) {
		*outptr++ = (unsigned char)c;
		return;
	}

	if (xfunc_out) xfunc_out((unsigned char)c);
}



/*----------------------------------------------*/
/* Put a null-terminated string                 */
/*----------------------------------------------*/

void xputs (					/* Put a string to the default device */
	const char* str				/* Pointer to the string */
)
{
	while (*str)
		xputc(*str++);
}


void xfputs (					/* Put a string to the specified device */
	void(*func)(unsigned char),	/* Pointer to the output function */
	const char*	str				/* Pointer to the string */
)
{
	void (*pf)(unsigned char);
	char *outptr_bak;


	outptr_bak = outptr;
	outptr = 0;
	pf = xfunc_out;		/* Save current output device */
	xfunc_out = func;	/* Switch output to specified device */
	while (*str)		/* Put the string */
		xputc(*str++);
	xfunc_out = pf;		/* Restore output device */
	outptr = outptr_bak;
}



/*----------------------------------------------*/
/* Formatted string output                      */
/*----------------------------------------------*/
/*  xprintf("%d", 1234);			"1234"
    xprintf("%6d,%3d%%", -200, 5);	"  -200,  5%"
    xprintf("%-6u", 100);			"100   "
    xprintf("%ld", 12345678L);		"12345678"
    xprintf("%04x", 0xA3);			"00a3"
    xprintf("%08LX", 0x123ABC);		"00123ABC"
    xprintf("%016b", 0x550F);		"0101010100001111"
    xprintf("%s", "String");		"String"
    xprintf("%-4s", "abc");			"abc "
    xprintf("%4s", "abc");			" abc"
    xprintf("%c", 'a');				"a"
    xprintf("%f", 10.0);            <xprintf lacks floating point support>
*/

#define _FLAG_0			0x0001 /* Flag: "0" padded */
#define _FLAG_LEFT		0x0002 /* Flag: Left justified */
#define _FLAG_LONG		0x0004 /* Flag: Long or Double */
#define _FLAG_SHORT		0x0008 /* Flag: Short */
#define _FLAG_TWICE		0x0010 /* Flag: LongLong, LongDouble(_FLAG_LONG), or Char(_FLAG_SHORT) */
#define _FLAG_SPACE		0x0020 /* Flag: add " "(space) instead of Sign */
#define _FLAG_SIGN		0x0040 /* Flag: add Sign(+ or -) */
#define _FLAG_PREFIX	0x0080 /* Flag: add Prefix */
#define _FLAG_MINUS		0x0100 /* Flag: Value is Minus */
#define _FLAG_FLOATING	0x0200 /* Flag: Value is Floating Point */
//#define _FLAG_LOWER		0x0400 /* Flag: Use lower case */
#define _FLAG_PRECISION	0x0800 /* Flag: Precision field existed */
//#define _FLAG_ROUND_UP	0x1000 /* Flag: Round up next value */
#define _FLAG_EXP_MINUS	0x2000 /* Flag: Exponent portion is minus */
//#define _FLAG_BITSHIFT	0x4000 /* Flag: Can use bit shift */
#define _FLAG_DENORMAL	0x8000 /* Flag: This is 0 or Denormal number */

#if _PROTECT_BUFFER
#define CHKBUF(num,buf) (num < sizeof(buf))
#else
#define CHKBUF(num,buf) (1)
#endif /* _PROTECT_BUFFER */

void xvprintf (
	const char*	fmt,	/* Pointer to the format string */
	va_list arp			/* Pointer to arguments */
)
{
	unsigned char r, i, j, w, o;
	unsigned short f;
	unsigned long v;
	char s[_BUFFER_SIZE], c, d, *p;
#if _USE_PRECISION || _USE_FLOATING
	unsigned char pr;
#endif /* _USE_PRECISION */
#if _USE_FLOATING
	short e;
#if _FLOATING_TYPE
#define _FLOATING_TYPEDEF	float
#else
#define _FLOATING_TYPEDEF	double
#endif /* _FLOATING_TYPE */
	_FLOATING_TYPEDEF vf;
#endif /* _USE_FLOATING */
#if _USE_ALL_LENGTH || _USE_FLOATING
	unsigned long long vl;
#endif /* _USE_ALL_LENGTH */
#if _USE_ALL_INT && !_DISABLE_N
	unsigned int cnt = 0;
#define _OUTPUT_COUNT(n)	(cnt += (n))
#else
#define _OUTPUT_COUNT(n)
#endif /* _USE_ALL_INT */
	for (;;) {
		c = *fmt++;					/* Get a char */
		if (!c) break;				/* End of format? */
		if (c != '%') {				/* Pass through it if not a % sequense */
			xputc(c); _OUTPUT_COUNT(1); continue;
		}
		f = 0;
#if _USE_ALL_FLAG
		while ((c = *fmt++)) {
			if (c == '0') {				/* Flag: '0' padded */
				f |= _FLAG_0; c = *fmt++;
				break;
			} else if (c == '-') {		/* Flag: left justified */
				f |= _FLAG_LEFT;
#if !_DISABLE_PLUS
			} else if (c == '+') {
				f |= _FLAG_SIGN;
#endif /* !_DISABLE_PLUS */
#if !_DISABLE_SPACE
			} else if (c == ' ') {
				f |= _FLAG_SPACE;
#endif /* !_DISABLE_SPACE */
#if !_DISABLE_SHARP
			} else if (c == '#') {
				f |= _FLAG_PREFIX;
#endif /* !_DISABLE_SHARP */
			} else {
				break;
			}
		}
#else
		c = *fmt++;					/* Get first char of the sequense */
		if (c == '0') {				/* Flag: '0' padded */
			f |= _FLAG_0; c = *fmt++;
		} else if (c == '-') {		/* Flag: left justified */
			f |= _FLAG_LEFT; c = *fmt++;
		}
#endif /* _USE_ALL_FLAG */
		for (w = 0; c >= '0' && c <= '9'; c = *fmt++)	/* Minimum width */
			w = w * 10 + c - '0';
#if _USE_PRECISION
		pr = 0;
		if (c == '.') {
			f |= _FLAG_PRECISION;
			c = *fmt++;
			for (; c >= '0' && c <= '9'; c = *fmt++)	/* Minimum width */
				pr = pr * 10 + c - '0';
		}
#endif /* _USE_PRECISION */
		if (c == 'l') {	/* Prefix: Size is long int */
			f |= _FLAG_LONG;
			c = *fmt++;
#if _USE_ALL_LENGTH
#if !_DISABLE_LL
			if (c == 'l') {
				f |= _FLAG_TWICE;
				c = *fmt++;
			}
#endif /* !_DISABLE_LL */
#if !_DISABLE_LONG_D
		} else if (c == 'L') {
			f |= _FLAG_TWICE;
			c = *fmt++;
#endif /* !_DISABLE_LONG_D */
#if !_DISABLE_H
		} else if (c == 'h') {
			f |= _FLAG_SHORT;
			c = *fmt++;
			if (c == 'h') {
				f |= _FLAG_TWICE;
				c = *fmt++;
			}
#endif /* !_DISABLE_H */
#endif /* _USE_ALL_LENGTH */
		}
		if (!c) break;				/* End of format? */
		d = c;
		if (d >= 'a') {
			d -= 0x20;
			o = 0x27;
		} else {
			o = 0x07;
		}
		switch (d) {				/* Type is... */
#if !_DISABLE_S
		case 'S' :					/* String */
			p = va_arg(arp, char*);
			for (j = 0; p[j]; j++) ;
#if _USE_PRECISION
			if (pr && pr < j) j = pr;
			c = j;
#endif /* _USE_PRECISION */
			if (!(f & _FLAG_LEFT)) while (j < w) {xputc(' '); j++;}
#if _USE_PRECISION
			while (c--) xputc(*(p++));
#else
			xputs(p);
#endif /* _USE_PRECISION */
			while (j < w) {xputc(' '); j++;}
			_OUTPUT_COUNT(j);
			continue;
#endif /* !_DISABLE_S */
#if !_DISABLE_C
		case 'C' :					/* Character */
			xputc((char)va_arg(arp, int)); _OUTPUT_COUNT(1); continue;
#endif /* !_DISABLE_C */
#if !_DISABLE_B
		case 'B' :					/* Binary */
			r = 2; break;
#endif /* !_DISABLE_B */
#if !_DISABLE_O
		case 'O' :					/* Octal */
			r = 8; break;
#endif /* !_DISABLE_O */
#if _USE_ALL_INT
#if !_DISABLE_I
		case 'I' :					/* Signed decimal (same as 'D') */
			d = 'D';
#endif /* !_DISABLE_I */
#endif /* _USE_ALL_INT */
		case 'D' :					/* Signed decimal */
		case 'U' :					/* Unsigned decimal */
			r = 10; break;
#if _USE_ALL_INT
#if !_DISABLE_N
		case 'N' :					/* save number of outputs */
			*((int*)va_arg(arp, int*)) = cnt; continue;
#endif /* !_DISABLE_N */
#if !_DISABLE_P
		case 'P' :					/* Pointer (same as "%#x") */
			f |= _FLAG_PREFIX;
#endif /* !_DISABLE_P */
#endif /* _USE_ALL_INT */
#if !_DISABLE_X
		case 'X' :					/* Hexdecimal */
			r = 16; break;
#endif /* !_DISABLE_X */
#if _USE_FLOATING
#if !_DISABLE_E
		case 'E' :
#endif /* !_DISABLE_E */
#if !_DISABLE_F
		case 'F' :
#endif /* !_DISABLE_F */
#if !_DISABLE_G
		case 'G' :
#endif /* !_DISABLE_G */
			r = 10; f |= _FLAG_FLOATING; break;
#if !_DISABLE_A
		case 'A' :
			r = 16; f |= _FLAG_FLOATING; break;
#endif /* !_DISABLE_A */
#endif /* _USE_FLOATING */
		default:					/* Unknown type (passthrough) */
			xputc(c); _OUTPUT_COUNT(1); continue;
		}

		i = 0;
		/* Get an argument and put it in numeral */
#if _USE_FLOATING
		if (f & _FLAG_FLOATING) {
			vf = (f & _FLAG_TWICE) ? (_FLOATING_TYPEDEF)va_arg(arp, long double) : (_FLOATING_TYPEDEF)va_arg(arp, double);
#if _FLOATING_TYPE
			v = ((*(unsigned long*)(&vf)) & 0x007FFFFF) << 1;
			e = (((int)((*(unsigned long*)(&vf)) >> 23)) & 0x00FF);
			if (((*(unsigned long*)(&vf)) & 0x80000000)) {
				(*(unsigned long*)(&vf)) &= 0x7FFFFFFF;
				f |= _FLAG_MINUS;
			}
			if (e == 0x00FF) { /* NaN or Inf */
				if (v) { /* NaN */
#else
			vl = ((*(unsigned long long*)(&vf)) & 0x000FFFFFFFFFFFFF);
			e = (((int)((*(unsigned long long*)(&vf)) >> 52)) & 0x07FF);
			if (((*(unsigned long long*)(&vf)) & 0x8000000000000000)) {
				(*(unsigned long long*)(&vf)) &= 0x7FFFFFFFFFFFFFFF;
				f |= _FLAG_MINUS;
			}
			if (e == 0x07FF) { /* NaN or Inf */
				if (vl) { /* NaN */
#endif /* _FLOATING_TYPE */
					s[i++] = 'n';
					s[i++] = 'a';
					s[i++] = 'n';
				} else { /* Inf */
					s[i++] = 'f';
					s[i++] = 'n';
					s[i++] = 'i';
				}
			} else {
#if !_DISABLE_A
				if (d == 'A') {
#if _FLOATING_TYPE
					if (e == 0) { /* 0 or Denormal number */
						f |= _FLAG_DENORMAL;
						if (v) e -= 0x007F;
					} else {
						e -= 0x007F;
					}
#else
					if (e == 0) { /* 0 or Denormal number */
						f |= _FLAG_DENORMAL;
						if (vl) e -= 0x03FF;
					} else {
						e -= 0x03FF;
					}
#endif /* _FLOATING_TYPE */
					if (e < 0) {
						e = -e;
						f |= _FLAG_EXP_MINUS;
					}
					do {
						s[i++] = '0' + (e % 10); e /= 10;
					} while (e && CHKBUF(i,s));
					s[i++] = (f & _FLAG_EXP_MINUS) ? '-' : '+';
					s[i++] = 'P' - 7 + o;
#if _USE_PRECISION
					if (f & _FLAG_PRECISION) {
						e = pr;
#if _FLOATING_TYPE
						while (pr > 6) {s[i++] = '0'; pr--;}
#if _USE_ROUND_UP
						v = (v >> (24 - (pr << 2))) + ((pr < 6) ? ((v >> (23 - (pr << 2)))&1) : 0);
#else
						v = (v >> (24 - (pr << 2)));
#endif /* _USE_ROUND_UP */
						while (pr-- && CHKBUF(i,s)) {
							c = (v & 0x0F); v = v >> 4;
							if (c > 9) c += o;
							s[i++] = c + '0';
						}
#else
						while (pr > 13) {s[i++] = '0'; pr--;}
#if _USE_ROUND_UP
						vl = (vl >> (52 - (pr << 2))) + ((pr < 13) ? ((vl >> (51 - (pr << 2)))&1) : 0);
#else
						vl = (vl >> (52 - (pr << 2)));
#endif /* _USE_ROUND_UP */
						while (pr-- && CHKBUF(i,s)) {
							c = (vl & 0x0F); vl = vl >> 4;
							if (c > 9) c += o;
							s[i++] = c + '0';
						}
#endif /* _FLOATING_TYPE */
					} else
#endif /* _USE_PRECISION */
					{
#if _FLOATING_TYPE
						pr = 6;
						if (v) while (!(v & 0x0F)) {v = v >> 4; pr--;}
						else pr = 0;
						e = pr;
						while (pr-- && CHKBUF(i,s)) {
							c = (v & 0x0F); v = v >> 4;
							if (c > 9) c += o;
							s[i++] = c + '0';
						}
#else
						pr = 13;
						if (vl) while (!(vl & 0x0F)) {vl = vl >> 4; pr--;}
						else pr = 0;
						e = pr;
						while (pr-- && CHKBUF(i,s)) {
							c = (vl & 0x0F); vl = vl >> 4;
							if (c > 9) c += o;
							s[i++] = c + '0';
						}
#endif /* _FLOATING_TYPE */
					}
					if (e || (f & _FLAG_PREFIX)) s[i++] = '.';
					s[i++] = (f & _FLAG_DENORMAL) ? '0' : '1';
					f |= _FLAG_PREFIX;
				} else
#endif /* !_DISABLE_A */
				{
#if _USE_PRECISION
					if (!(f & _FLAG_PRECISION)) pr = 6;
#else
					pr = 6;
#endif /* _USE_PRECISION */
#if !_DISABLE_G
					if (d == 'G' && pr) pr--; // %G -> %E
#endif /* !_DISABLE_G */
					e = 0;
#if !_DISABLE_E || !_DISABLE_G
					if (d != 'F') {
						while (vf < 1) {vf *= 10; e--;}
						while (vf >= 10) {vf *= (_FLOATING_TYPEDEF)0.1; e++;}
						if (e < 0) {
							e = -e;
							f |= _FLAG_EXP_MINUS;
						}
						s[i++] = '0' + (char)(e % 10); e /= 10;
						do {
							s[i++] = '0' + (char)(e % 10); e /= 10;
						} while (e);
						s[i++] = (f & _FLAG_EXP_MINUS) ? '-' : '+';
						s[i++] = 'E' - 7 + o;
#if _USE_FAST_FLOAT
#if _USE_ROUND_UP
						v = 10;
						e = pr;
						while (e--) v *= 10;
						v *= vf;
						if (((v % 10) << 1) >= 10) v += 10; /* Round up */
						v /= 10;
#else
						v = 1;
						e = pr;
						while (e--) v *= 10;
						v *= vf;
#endif /* _USE_ROUND_UP */
						e = pr;
						while (e-- && CHKBUF(i,s)) {
							s[i++] = '0' + (char)(v % 10); v /= 10;
						}
						if (pr || (f & _FLAG_PREFIX)) s[i++] = '.';
#if _USE_ROUND_UP
						if (v >= 10) {s[i++] = '0'; s[i++] = '1';}
						else {s[i++] = '0' + v;}
#else
						s[i++] = '0' + v;
#endif /* _USE_ROUND_UP */
#endif /* _USE_FAST_FLOAT */
					}
#if _USE_FAST_FLOAT
					else
#endif /* _USE_FAST_FLOAT */
#endif /* !_DISABLE_E || !_DISABLE_G */
					{
#if _USE_ROUND_UP
						v = 10;
						e = pr;
						while (e--) v *= 10;
						v *= (vf - (unsigned long)vf);
						if (((v % 10) << 1) >= 10) v += 10; /* Round up */
						v /= 10;
#else
						v = 1;
						e = pr;
						while (e--) v *= 10;
						v *= (vf - (unsigned long)vf);
#endif /* _USE_ROUND_UP */
						e = pr;
						while (e-- && CHKBUF(i,s)) {
							s[i++] = '0' + (char)(v % 10); v /= 10;
						}
						if (pr || (f & _FLAG_PREFIX)) s[i++] = '.';
#if _USE_ROUND_UP
						v += ((unsigned long)vf);
#else
						v = ((unsigned long)vf);
#endif /* _USE_ROUND_UP */
						do {
							s[i++] = '0' + (char)(v % 10); v /= 10;
						} while (v && CHKBUF(i,s));
					}
				}
			}
		} else
#endif /* _USE_FLOATING */
#if _USE_ALL_LENGTH
#if !_DISABLE_LL
		if ((f & _FLAG_LONG) && (f & _FLAG_TWICE)) {
#if _USE_FAST_LL
			if (r == 10) {
				vl = va_arg(arp, long long);
				if (d == 'D' && (vl & 0x8000000000000000ull)) {
					vl = 0ull - vl;
					f |= _FLAG_MINUS;
				}
				int m;
				m = 7;
				v = vl % 10000000; vl /= 10000000;
				do {
					s[i++] = '0' + (char)(v % 10); v /= 10;
				} while (((--m && vl) || v) && CHKBUF(i,s));
				m = 7;
				v = vl % 10000000; vl /= 10000000;
				while (((m-- && vl) || v) && CHKBUF(i,s)) {
					s[i++] = '0' + (char)(v % 10); v /= 10;
				}
				m = 7;
				v = vl;
				while ((m-- && v) && CHKBUF(i,s)) {
					s[i++] = '0' + (char)(v % 10); v /= 10;
				}
			} else {
				vl = va_arg(arp, unsigned long long);
				switch (r) {
#if !_DISABLE_X
				case 16:
					v = (unsigned long)vl;
					do {
						c = (v & 0x0F); v = v >> 4;
						if (c > 9) c += o;
						s[i++] = c + '0';
					} while (v && CHKBUF(i,s));
					v = vl >> 32;
					if (v) while (i < 8) s[i++] = '0';
					while (v && CHKBUF(i,s)) {
						c = (v & 0x0F); v = v >> 4;
						if (c > 9) c += o;
						s[i++] = c + '0';
					}
					break;
#endif /* !_DISABLE_X */
#if !_DISABLE_B
				case 2:
					v = (unsigned long)vl;
					do {
						s[i++] = '0'+ (v & 1); v = v >> 1;
					} while (v && CHKBUF(i,s));
					v = vl >> 32;
					if (v) while (i < 32) s[i++] = '0';
					while (v && CHKBUF(i,s)) {
						s[i++] = '0'+ (v & 1); v = v >> 1;
					}
					break;
#endif /* !_DISABLE_B */
#if !_DISABLE_O
				default:
					v = (unsigned long)vl;
					do {
						s[i++] = '0'+ (v & 0x07); v = v >> 3;
					} while (v && CHKBUF(i,s));
					v = vl >> 32;
					if (v) {
						while (i < 11) s[i++] = '0';
						s[i - 1] += v & 1; v = v >> 1;
					}
					while (v && CHKBUF(i,s)) {
						s[i++] = '0'+ (v & 0x07); v = v >> 3;
					}
					break;
#endif /* !_DISABLE_O */
				}
			}
#else
			vl = va_arg(arp, long long);
			if (d == 'D' && (vl & 0x8000000000000000ull)) {
				vl = 0ull - vl;
				f |= _FLAG_MINUS;
			}
			do {
				c = (char)(vl % r); vl /= r;
				if (c > 9) c += o;
				s[i++] = c + '0';
			} while (vl && CHKBUF(i,s));
#endif /* _USE_FAST_LL */
#if _USE_PRECISION
			if (pr) while (i < pr) s[i++] = '0';
			else if ((f &_FLAG_PRECISION) && !c && i == 1) i = 0;
#endif /* _USE_PRECISION */
		} else
#endif /* !_DISABLE_LL */
#endif /* _USE_ALL_LENGTH */
		{
#if _USE_FAST_LONG
			if (r == 10) {
				v = (f & _FLAG_LONG) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int));
				if (d == 'D' && (v & 0x80000000ul)) {
					v = 0 - v;
					f |= _FLAG_MINUS;
				}
				do {
					s[i++] = '0' + (char)(v % 10); v /= 10;
				} while (v && CHKBUF(i,s));
			} else {
				v = (f & _FLAG_LONG) ? (long)va_arg(arp, unsigned long) : (long)va_arg(arp, unsigned int);
				switch (r) {
#if !_DISABLE_X
				case 16:
					do {
						c = ((unsigned int)v & 0x0F); v = v >> 4;
						if (c > 9) c += o;
						s[i++] = c + '0';
					} while (v && CHKBUF(i,s));
					break;
#endif /* !_DISABLE_X */
#if !_DISABLE_B
				case 2:
					do {
						s[i++] = '0' + ((unsigned int)v & 1); v = v >> 1;
					} while (v && CHKBUF(i,s));
					break;
#endif /* !_DISABLE_B */
#if !_DISABLE_O
				default:
					do {
						s[i++] = '0' + ((unsigned int)v & 0x07); v = v >> 3;
					} while (v && CHKBUF(i,s));
					break;
#endif /* !_DISABLE_O */
				}
			}
#else
			v = (f & _FLAG_LONG) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int));
			if (d == 'D' && (v & 0x80000000ul)) {
				v = 0 - v;
				f |= _FLAG_MINUS;
			}
			do {
				c = (char)(v % r); v /= r;
				if (c > 9) c += o;
				s[i++] = c + '0';
			} while (v && CHKBUF(i,s));
#endif /* _USE_FAST_LONG */
#if _USE_PRECISION
			if (pr) while (i < pr) s[i++] = '0';
			else if ((f & _FLAG_PRECISION) && !c && i == 1) i = 0;
#endif /* _USE_PRECISION */
		}
		d = c = 0;
		if (f & _FLAG_MINUS) c = '-';
#if _USE_ALL_FLAG
#if !_DISABLE_O
		else if ((f & _FLAG_PREFIX) && r == 8) c = '0';
#endif /* !_DISABLE_O */
#if !_DISABLE_PLUS
		else if (f & _FLAG_SIGN) c = '+';
#endif /* !_DISABLE_PLUS */
#if !_DISABLE_SPACE
		else if (f & _FLAG_SPACE) c = ' ';
#endif /* !_DISABLE_SPACE */
#endif /* _USE_ALL_FLAG */
		j = 0;
		if (f & _FLAG_0) {
			if (c) {
				xputc(c); j++;
			}
#if _USE_ALL_FLAG || _USE_FLOATING
#if (!_DISABLE_X && !_DISABLE_SHARP) || !_DISABLE_A
			if ((f & _FLAG_PREFIX) && r == 16) {xputc('0'); xputc('X' - 7 + o); j += 2;}
#endif /* (!_DISABLE_X && !_DISABLE_SHARP) || !_DISABLE_A */
#endif /* _USE_ALL_FLAG || _USE_FLOATING */
			c = '0';
		} else {
#if _USE_ALL_FLAG || _USE_FLOATING
#if (!_DISABLE_X && !_DISABLE_SHARP) || !_DISABLE_A
			if ((f & _FLAG_PREFIX) && r == 16) {s[i++] = ('X' - 7 + o); s[i++] = '0';}
#endif /* (!_DISABLE_X && !_DISABLE_SHARP) || !_DISABLE_A */
#endif /* _USE_ALL_FLAG || _USE_FLOATING */
			if (c) {
				s[i++] = c;
			}
			c = ' ';
		}
		j += i;
		if (!(f & _FLAG_LEFT)) while (j < w) {xputc(c); j++;}
		while (i--) xputc(s[i]);
		while (j < w) {xputc(' '); j++;}
		_OUTPUT_COUNT(j);
	}
}


void xprintf (			/* Put a formatted string to the default device */
	const char*	fmt,	/* Pointer to the format string */
	...					/* Optional arguments */
)
{
	va_list arp;


	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);
}

#if _USE_VA_LIST
#if _GET_LENGTH
int
#else
void
#endif
xvsprintf (				/* Put a formatted string to the memory */
	char* buff,			/* Pointer to the output buffer */
	const char*	fmt,	/* Pointer to the format string */
	va_list arp			/* Pointer to arguments */
)
{
	char *outptr_bak;


	outptr_bak = outptr;
	outptr = buff;		/* Switch destination for memory */

	xvprintf(fmt, arp);

#if _GET_LENGTH
	buff = (char *) (outptr - buff);
#endif

	*outptr = 0;		/* Terminate output string with a \0 */
	outptr = outptr_bak;			/* Switch destination for device */

#if _GET_LENGTH
	return (int)buff;
#endif
}

void xvfprintf (					/* Put a formatted string to the specified device */
	void(*func)(unsigned char),	/* Pointer to the output function */
	const char*	fmt,			/* Pointer to the format string */
	va_list arp					/* Pointer to arguments */
)
{
	void (*pf)(unsigned char);
	char *outptr_bak;


	outptr_bak = outptr;
	outptr = 0;
	pf = xfunc_out;		/* Save current output device */
	xfunc_out = func;	/* Switch output to specified device */

	xvprintf(fmt, arp);

	xfunc_out = pf;		/* Restore output device */
	outptr = outptr_bak;
}
#endif

#if _GET_LENGTH
int
#else
void
#endif
xsprintf (			/* Put a formatted string to the memory */
	char* buff,			/* Pointer to the output buffer */
	const char*	fmt,	/* Pointer to the format string */
	...					/* Optional arguments */
)
{
	va_list arp;
	char *outptr_bak;


	outptr_bak = outptr;
	outptr = buff;		/* Switch destination for memory */

	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);

#if _GET_LENGTH
	buff = (char *) (outptr - buff);
#endif

	*outptr = 0;		/* Terminate output string with a \0 */
	outptr = outptr_bak;			/* Switch destination for device */

#if _GET_LENGTH
	return (int)buff;
#endif
}

void xfprintf (					/* Put a formatted string to the specified device */
	void(*func)(unsigned char),	/* Pointer to the output function */
	const char*	fmt,			/* Pointer to the format string */
	...							/* Optional arguments */
)
{
	va_list arp;
	void (*pf)(unsigned char);
	char *outptr_bak;


	outptr_bak = outptr;
	outptr = 0;
	pf = xfunc_out;		/* Save current output device */
	xfunc_out = func;	/* Switch output to specified device */

	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);

	xfunc_out = pf;		/* Restore output device */
	outptr = outptr_bak;
}

/*----------------------------------------------*/
/* Dump a line of binary dump                   */
/*----------------------------------------------*/

void put_dump (
	const void* buff,		/* Pointer to the array to be dumped */
	unsigned long addr,		/* Heading address value */
	int len,				/* Number of items to be dumped */
	int width				/* Size of the items (DF_CHAR, DF_SHORT, DF_LONG) */
)
{
	int i;
	const unsigned char *bp;
	const unsigned short *sp;
	const unsigned long *lp;


	xprintf("%08lX ", addr);		/* address */

	switch (width) {
	case DW_CHAR:
		bp = buff;
		for (i = 0; i < len; i++)		/* Hexdecimal dump */
			xprintf(" %02X", bp[i]);
		xputc(' ');
		for (i = 0; i < len; i++)		/* ASCII dump */
			xputc((bp[i] >= ' ' && bp[i] <= '~') ? bp[i] : '.');
		break;
	case DW_SHORT:
		sp = buff;
		do								/* Hexdecimal dump */
			xprintf(" %04X", *sp++);
		while (--len);
		break;
	case DW_LONG:
		lp = buff;
		do								/* Hexdecimal dump */
			xprintf(" %08LX", *lp++);
		while (--len);
		break;
	}

	xputc('\n');
}

#endif /* _USE_XFUNC_OUT */



#if _USE_XFUNC_IN
unsigned char (*xfunc_in)(void);	/* Pointer to the input stream */

/*----------------------------------------------*/
/* Get a line from the input                    */
/*----------------------------------------------*/

int xgets (		/* 0:End of stream, 1:A line arrived */
	char* buff,	/* Pointer to the buffer */
	int len		/* Buffer length */
)
{
	int c, i;


	if (!xfunc_in) return 0;		/* No input function specified */

	i = 0;
	for (;;) {
		c = xfunc_in();				/* Get a char from the incoming stream */
		if (!c) return 0;			/* End of stream? */
		if (c == '\r') break;		/* End of line? */
		if (c == '\b' && i) {		/* Back space? */
			i--;
			if (_LINE_ECHO) xputc(c);
			continue;
		}
		if (c >= ' ' && i < len - 1) {	/* Visible chars */
			buff[i++] = c;
			if (_LINE_ECHO) xputc(c);
		}
	}
	buff[i] = 0;	/* Terminate with a \0 */
	if (_LINE_ECHO) xputc('\n');
	return 1;
}


int xfgets (	/* 0:End of stream, 1:A line arrived */
	unsigned char (*func)(void),	/* Pointer to the input stream function */
	char* buff,	/* Pointer to the buffer */
	int len		/* Buffer length */
)
{
	unsigned char (*pf)(void);
	int n;


	pf = xfunc_in;			/* Save current input device */
	xfunc_in = func;		/* Switch input to specified device */
	n = xgets(buff, len);	/* Get a line */
	xfunc_in = pf;			/* Restore input device */

	return n;
}


/*----------------------------------------------*/
/* Get a value of the string                    */
/*----------------------------------------------*/
/*	"123 -5   0x3ff 0b1111 0377  w "
	    ^                           1st call returns 123 and next ptr
	       ^                        2nd call returns -5 and next ptr
                   ^                3rd call returns 1023 and next ptr
                          ^         4th call returns 15 and next ptr
                               ^    5th call returns 255 and next ptr
                                  ^ 6th call fails and returns 0
*/

int xatoi (			/* 0:Failed, 1:Successful */
	char **str,		/* Pointer to pointer to the string */
	long *res		/* Pointer to the valiable to store the value */
)
{
	unsigned long val;
	unsigned char c, r, s = 0;


	*res = 0;

	while ((c = **str) == ' ') (*str)++;	/* Skip leading spaces */

	if (c == '-') {		/* negative? */
		s = 1;
		c = *(++(*str));
	}

	if (c == '0') {
		c = *(++(*str));
		switch (c) {
		case 'x':		/* hexdecimal */
			r = 16; c = *(++(*str));
			break;
		case 'b':		/* binary */
			r = 2; c = *(++(*str));
			break;
		default:
			if (c <= ' ') return 1;	/* single zero */
			if (c < '0' || c > '9') return 0;	/* invalid char */
			r = 8;		/* octal */
		}
	} else {
		if (c < '0' || c > '9') return 0;	/* EOL or invalid char */
		r = 10;			/* decimal */
	}

	val = 0;
	while (c > ' ') {
		if (c >= 'a') c -= 0x20;
		c -= '0';
		if (c >= 17) {
			c -= 7;
			if (c <= 9) return 0;	/* invalid char */
		}
		if (c >= r) return 0;		/* invalid char for current radix */
		val = val * r + c;
		c = *(++(*str));
	}
	if (s) val = 0 - val;			/* apply sign if needed */

	*res = val;
	return 1;
}

#endif /* _USE_XFUNC_IN */
