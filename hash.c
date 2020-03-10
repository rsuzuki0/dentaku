/*
 *  hash.c: Hash file routin of soroban(terakoya-system)
 *  version 1.00 1989/11/25 released on 1989/11/25
 *	programmed by JK1LOT(Dai Yokota)
 *	Copyright 1989 (C) by JK1LOT(Dai Yokota)
 *	All rights are reserved
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <io.h>
#include <time.h>
#include <fcntl.h>
#include <sys\stat.h>
#include "dentaku.h"
#include "hash.h"

#define FALSE 0
#define TRUE 1

static FILE *datafp;
static int hashhandle;
static char datafn[MAXPATH], hashfn[MAXPATH];
static char cachebuf[BUFSIZ];

int
init_hash(char *datafile, char *hashfile)
{
	*cachebuf = '\0';
	strcpy(datafn, datafile);
	if ((datafp = fopen(datafile, "at+")) == NULL)
		return FALSE;
	rewind(datafp);
	strcpy(hashfn, hashfile);
	if ((hashhandle =
	   open(hashfile, O_RDWR|O_CREAT|O_BINARY, S_IREAD|S_IWRITE)) == -1)
		return FALSE;
	return TRUE;
}

void
close_hash()
{
	fclose(datafp);
	close(hashhandle);
}

static int
creat_init_hash()
{
	register int i;
	long l;

	l = -1L;
	close(hashhandle);
	remove(hashfn);
	if ((hashhandle
	 = open(hashfn, O_RDWR|O_CREAT|O_BINARY, S_IREAD|S_IWRITE)) == -1)
		return FALSE;
	for (i = 0; i < HASHSIZE; i++) {
		write(hashhandle, &l, sizeof(l));
		write(hashhandle, &l, sizeof(l));
	}
	return TRUE;
}

int
creat_hash(char *datatmp)
{
	FILE *dfp, *tfp;
	char buf[BUFSIZ];

	creat_init_hash();

	fclose(datafp);
	remove(datatmp);
	if ((dfp = fopen(datafn, "rt")) == NULL)
		return FALSE;
	if ((tfp = fopen(datatmp, "wt")) == NULL)
		return FALSE;
	while (fgets(buf, BUFSIZ, dfp) != NULL) {
		fputs(buf, tfp);
	}
	fclose(dfp);
	fclose(tfp);
	remove(datafn);
	if ((datafp = fopen(datafn, "at+")) == NULL)
		return FALSE;
	if ((tfp = fopen(datatmp, "rt")) == NULL)
		return FALSE;

	while(fgets(buf, BUFSIZ, tfp) != NULL) {
		add_hash(buf);
	}
	fclose(tfp);
	remove(datatmp);

	return TRUE;
}

void
add_hash(char *string)
{
	char key[BUFSIZ];
	char *p, *q;
	long hashp, datap, newhashp;
	long empty = -1L;

/*
	strcpy(cachebuf, string);
*/

	for (p = string, q = key; (*p != ' ')&& *p; p++, q++)
		*q = *p;
	*q = '\0';

	hashp = (long)hash(key) * sizeof(long) * 2;
	for (;;) {
		lseek(hashhandle, hashp, SEEK_SET);
		read(hashhandle, &datap, sizeof(datap));
		if (datap == -1) break;
		read(hashhandle, &hashp, sizeof(hashp));
	}
	fseek(datafp, 0L, SEEK_END);
	datap = ftell(datafp);
	fputs(string, datafp);

	lseek(hashhandle, 0L, SEEK_END);
	newhashp = tell(hashhandle);
	write(hashhandle, &empty, sizeof(empty));
	write(hashhandle, &empty, sizeof(empty));

	lseek(hashhandle, hashp, SEEK_SET);
	write(hashhandle, &datap, sizeof(datap));
	write(hashhandle, &newhashp, sizeof(newhashp));
}

char *
search_hash(char *key, char *buf)
{
	long hashp, datap;
	int keylen;

	keylen = strlen(key);
	if (!strncmp(key, cachebuf, keylen)) {
		strcpy(buf, cachebuf);
		return buf;
	}

	hashp = (long)hash(key) * sizeof(long) * 2;

	for (;;) {
		lseek(hashhandle, hashp, SEEK_SET);
		read(hashhandle, &datap, sizeof(datap));
		if (datap == -1) break;
		fseek(datafp, datap, SEEK_SET);
		fgets(buf, BUFSIZ, datafp);
		if (!(strncmp(key, buf, keylen))) break;
		read(hashhandle, &hashp, sizeof(hashp));
	}
	if (datap == -1) return NULL;
	strcpy(cachebuf, buf);
	return buf;
}

int
hash(const char *string)
{
	register unsigned char const *p;
	register unsigned int retval;
	register unsigned flag = 0;
	
	retval = 0x5555;
	if (*string == '<') p = string + 1;
	else p = string; /* You cannot be too careful! */
	for (; !(*p == '.' && flag) && *p != '>' && *p; p++) {
		if (*p == '@') {
			flag = 1;
		} else if (flag) {
			retval <<= 1;
			retval ^= (*p & 31);
		} else {
			retval <<= 3;
			retval++;
			retval ^= ((*p - 7 & 127));
		}
	}
	retval >>= 2;
	retval &= 4095;
	return retval;
}
