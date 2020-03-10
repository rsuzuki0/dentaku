/* ryujilib.[ch] は、wexlib.[ch] と違ってより汎用的なモジュールを */
/* 収集したものです。DENTAKU の著作権は主張します。*/
/* */
/* version 0.00  28th  Jun, 1992 Ryuji Suzuki(JF7WEX). */

#include <stdio.h>
#include "ryujilib.h"

/* strtoke() is like strtok(). */
char *
strtoke(char *src, const char *sep)
{
	register char const *r;
	register char *q;
	char *p;
	static char *srcptr;

	if (!(srcptr || src ) || !sep) {
		srcptr = NULL;
		return NULL;
	}
	if (src) {
		srcptr = src;
		p = src;
	} else {
		p = srcptr;
	}
	for (q = p; *q; q++) {
		for (r = sep; *r; r++) {
			if (*q == *r) {
				*q = NULL;
				if (*++q != '\0') {
					srcptr = q;
				} else {
					srcptr = NULL;
				}
				return p;
			}
		}
	}
	return p;
}


/* strcntchr counts that how many key characters are included in src string. */
unsigned
strcntchr(const char *src, char key)
{
	register unsigned i;
	register unsigned char j;
	register unsigned n = 0;

	for (i = 0; (j = src[i]) != (char)'\0'; i++) {
		if (j == key) n++;
	}
	return n;
}


/* fgetsb() is like fgets() but ignores CR */
char *
fgetsb(char buf[], int n, FILE *fp)
{
	register unsigned i;
	unsigned char c;

	buf[--n] = '\0';
	for (i = 0; i <= n; i++) {
		if (1 != fread(&c, 1, 1, fp)) {
			buf[i] = '\0';
			return NULL;
		} else buf[i] = c;
		if (c == 0x0a) {
			buf[++i] = '\0';
			break;
		}
	}
	buf[i] = '\0';
	return buf;
}
