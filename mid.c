/*
 *  mid.c: Message-Id 管理 routin of soroban(terakoya-system)
 *  version 2.00 1989/11/25 released on 1989/11/25
 *	programmed by JK1LOT(Dai Yokota)
 *	Copyright 1989 (C) by JK1LOT(Dai Yokota)
 *	All rights are reserved
 *  18 April, 1992 jf7wex(Ryuji Suzuki) (format of midfile)
 *
 */

/* このファイルは soroban に使われている mid.c を dentaku に使うために */
/* 一部修正させていただいたもので、特に dentaku に不要な関数は */
/* 取り除いてあり、新たに関数を作ったものもあります。 */
/* 一応、大部分のところは マクロ ORIGINAL を定義すればオリジナルと */
/* 同じになりますが、完全に同じにはならないので soroban に使う時は */
/* soroban のパッケージに含まれるものをお使いください。 */
/* また、soroban に含まれる mid.c はその後更新されましたが */
/* dentaku に含まれる mid.c では必要がないという理由で */
/* 更新されていないコードを含んでいます。 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dir.h>
#include <string.h>
#include <io.h>
#include <sys/stat.h>
#ifdef ORIGINAL
#include "soroban.h"
#else
#include "dentaku.h"
#endif
#include "hash.h"
#include "errors.h"
#include "tempfile.h"
#include "wexlib.h"

#ifdef ORIGINAL
extern char newsdir[];
#endif

void
init_mid(int mode)
{
	if (mode == CONFIG) {
		init_hash(mid_file, mid_idx);
		config_mid();
		close_hash();
	} else {
		if ((access(mid_idx, 0) == -1) || (filetimecomp(mid_file, mid_idx) > 0)) {
			errprintf("%s: index file history.idx too old or not existed\n"
			  "\tSo now create new index file\n", comname);
			init_hash(mid_file, mid_idx);
			config_mid();
			close_hash();
		}
	}
	if (!init_hash(mid_file, mid_idx)) {
		errprintf("%s: history can't open\n", comname);
		exit(1);
	}
}

void
close_mid()
{
	close_hash();
}

void
config_mid()
{
	char *tmpfile;
	FILE *fp;
	
	fp = tfcreat(&tmpfile);
	tfclose(fp);

	if (!creat_hash(tmpfile)) {
		errprintf("%s: create index file Failed\n", comname);
		exit(1);
	}
	tfremove(tmpfile);
}

/* 一応使わないし、誤って使うと危険なのでコメントアウト */
#ifdef ORIGINAL
void
add_mid(char *mid, char *filename)
{
	char buf2[BUFSIZ];
	time_t timer;

	time(&timer);
	if (strncmp(filename, newsdir, newslen - 1)) {
		sprintf(buf2, "%s %lu %s\n", mid, timer, filename);
	} else {
		sprintf(buf2, "%s %lu %s\n", mid, timer, filename + newslen);
	}
	add_hash(buf2);
}


int
srch_mid(char *mid)
{
	char buf[BUFSIZ];

	if (search_hash(mid, buf) == NULL) return FALSE;
	return TRUE;
}

FILE *
midopen(char *mid, char *mode)
{
	char buf[BUFSIZ];
	char buf2[BUFSIZ];

	if (search_hash(mid, buf) == NULL) return NULL;

	strtok(buf, " \t\n");  /* mid */
	strtok(NULL, " \t\n"); /* time */
	strcpy(buf2, newsdir);
	strcat(buf2, "/");
	strcat(buf2, strtok(NULL, " \t\n"));
	return (fopen(buf2, mode));
}
#endif

#ifdef ORIGINAL
#else
/* message-id から処理された時刻を取得する */
long
midtime(char *mid)
{
	char buf[BUFSIZ];
	char *ptr;

	if (search_hash(mid, buf) == NULL) return NULL;
	strtok(buf, " \t\n");  /* mid */
	ptr = strtok(NULL, " \t\n");
	if (strchr(ptr, '/')) {
		return cvoldtime(ptr, strtok(NULL, " \t\n"));
	} else {
		return atol(ptr);
	}
}
#endif

/* これも */
#ifdef ORIGINAL
int
midrm(char *mid)
{
	char buf[BUFSIZ];
	char buf2[BUFSIZ];

	if (search_hash(mid, buf) == NULL) return FALSE;
	strtok(buf, " \t\n");  /* mid */
	strtok(NULL, " \t\n"); /* time */
	strcpy(buf2, newsdir);
	strcat(buf2, "/");
	strcat(buf2, strtok(NULL, " \t\n"));
	remove(buf2);
	return TRUE;
}

FILE *
midlckopen(char *mid, char *mode)
{
	char lckname[MAXPATH];
	char buf[BUFSIZ];
	char buf2[BUFSIZ];

	if (search_hash(mid, buf) == NULL) return NULL;

	strtok(buf, " \t\n");  /* mid */
	strtok(NULL, " \t\n"); /* time */
	sprintf(lckname, "%s/%s.lck", newsdir, strtok(NULL, " \t\n"));
	return (fopen(lckname, mode));
}

void
midlckrm(char *mid)
{
	char lckname[MAXPATH];
	char buf[BUFSIZ];

	if (search_hash(mid, buf) == NULL) return;

	strtok(buf, " \t\n");  /* mid */
	strtok(NULL, " \t\n"); /* time */
	sprintf(lckname, "%s/%s.lck", newsdir, strtok(NULL, " \t\n"));
	remove(lckname);
}

#endif

long
filetimecomp(char *fn1, char *fn2)
{
	struct stat stat1, stat2;

	stat(fn1, &stat1);
	stat(fn2, &stat2);
	return (stat1.st_mtime - stat2.st_mtime);
}
