/*
 *	kanji.c:kanji i/o routin to convert kanji code.
 *		shift jis -> new jis
 *	version 1.01 1989/03/20
 *	copyright 1988, 1989 (C) by Shigeki Matsushima
 *	all rights are reserved
 *	programmed by Shigeki Matsushima
 */

#include <stdio.h>
#include <stdlib.h>
#include <jctype.h>

#define TRUE	1
#define FALSE	0

void
fputsNJ(unsigned char *buf, FILE *fp)
{
	register int Kanji = FALSE;
	unsigned char hi, lo;

	while (*buf) {
		if (iskanji(*buf) && iskanji2(*(buf+1))) {
			if (Kanji == FALSE) {
				Kanji = TRUE;
				fputs("\033$B", fp);
			}
			hi = *buf++;
			if((lo = *buf++)=='\0')
				break;
			hi -= (hi <= 0x9f) ? 0x71 : 0xb1;
			hi = hi * 2 + 1;
			if (lo > 0x7f) lo -= 1;
			if (lo >= 0x9e) {
				lo -= 0x7d;
				hi += 1;
			} else lo -= 0x1f;
			fputc(hi, fp);
			fputc(lo, fp);
		} else {
			if (Kanji == TRUE) {
				Kanji = FALSE;
				fputs("\033(J", fp);
			}
			fputc(*buf++, fp);
		}
	}
}
