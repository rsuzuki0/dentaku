/*
 *	errors.c: エラー処理ルーチン
 *  version 1.20 1990/7/4
 *	programmed by JK1LOT(Dai Yokota)
 *	Copyright 1988,89,90 (C) by JK1LOT(Dai Yokota)
 *	All rights are reserved
 */

#include <stdio.h>
#include <stdlib.h>		/* ms-dos */
#include <time.h>
#include <stdarg.h>
#include "errors.h"
#include "dentaku.h"

extern int lines_oimorc; /* current line No. of oimo.rc */
extern char bufsave[]; /* current lines of oimo.rc */

FILE *errlogfp;

/* メモリ不足エラー処理 */
void alloc_error(char *mesg)
{
	errprintf("%s(%s()):Not enough memory\n", comname, mesg);
	exit(1);
}

/* DISK error */
void disk_error(char *fn)
{
	errprintf("%s(%s()):Disk error(maybe DISK full)\n", comname, fn);
	exit(1);
}

void
errprintf(char *fmt,...)
{
	char buf[BUFSIZ];
	time_t timer;

	va_list args;

	va_start(args,fmt);
	vsprintf(buf,fmt,args);
	(void)time(&timer);
	fputs(ctime(&timer), errlogfp);
	fputs(buf, errlogfp);
	fputs(buf, stderr);
	va_end(args);
}
