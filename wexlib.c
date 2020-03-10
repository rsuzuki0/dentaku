/* 寺子屋コマンド用汎用関数集 */
/* このライブラリを寺子屋コマンド作成に使用する際の条件等は dentaku の */
/* それに準じます。寺子屋以外に使用する際はご相談ください。 */

/* version 0.00     May 2nd  1992 Ryuji Suzuki(JF7WEX). */
/* version 0.02     Jun 21st 1992 Ryuji Suzuki(JF7WEX). */
/* version 0.03     Jun 28th 1992 Ryuji Suzuki(JF7WEX). */

#ifdef __TURBOC__
#include <alloc.h>
#else
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <dos.h>
#ifdef __TURBOC__
#include <dir.h>
#else
#include <direct.h>
#endif
#include "wexlib.h"
#include "ryujilib.h"

/* cvoldtime converts OLD STYLED time format that is used on sys to be UNIX */
/* time fotmat, time_t type. */
/* June 1992, Some bugs were fixed. (Thanks for JP1FOC's indication.) */
time_t
cvoldtime(unsigned char *day, unsigned char *time)
{
	register time_t converted;
	register int i;
	int month;
	char *ptr;
	const static unsigned monthtbl[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	day[2] = (char)NULL;
	i = atoi(day);
	converted = ((long)(i - 70) * 60L * 60L * 24L * 365L) + (long)((i - 69) / 4) * 60L * 60L * 24L;

	ptr = day + 3;
	*(ptr + 3) = (char)NULL;

	switch (*ptr) {
		case 'J':
		case 'j':
			ptr++;
			if (!strncmp(ptr, "an", 2)) {
				month = 1;
				break;
			} else if (!strncmp(ptr, "un", 2)) {
				month = 6;
				break;
			} else if (!strncmp(ptr, "ul", 2)) {
				month = 7;
				break;
			}
			return 0;
		case 'A':
		case 'a':
			ptr++;
			if (!strncmp(ptr, "pr", 2)) {
				month = 4;
				break;
			} else if (!strncmp(ptr, "ug", 2)) {
				month = 8;
				break;
			}
			return 0;
		case 'M':
		case 'm':
			ptr++;
			if (!strncmp(ptr, "ar", 2)) {
				month = 3;
				break;
			} else if(!strncmp(ptr, "ay", 2)) {
				month = 5;
				break;
			}
			return 0;
		default :
			if (!strnicmp(ptr, "Feb", 3))
				month = 2;
			else if (!strnicmp(ptr, "Sep", 3))
				month = 9;
			else if (!strnicmp(ptr, "Oct", 3))
				month = 10;
			else if (!strnicmp(ptr, "Nov", 3))
				month = 11;
			else if (!strnicmp(ptr, "Dec", 3))
				month = 12;
			else return 0;
			++ptr;
			break;
	}
	if (month >= 3 && 0 == i % 4) converted += 60L * 60L * 24L;
	--month;
	while (((signed)--month) >= 0) {
		converted += (long)monthtbl[month] * 60L * 60L * 24L;
	}
	ptr += 3;
	converted += (long)(atoi(ptr) - 1) * 60L * 60L * 24L;

	ptr = time;
	*(ptr + 2) = (char)NULL;
	converted += 60L * 60L * (long)atoi(ptr);

	ptr += 3;
	*(ptr + 2) = (char)NULL;
	converted += 60L * (long)atoi(ptr);

	ptr += 3;
	*(ptr + 2) = (char)NULL;
	converted += atoi(ptr);

	converted -= 60L * 60L * 9L;
	return converted;
}

struct wrk *wrktop;

/* scan spoolDir/mqueue/*.wrk */
void
scanwrkfile(unsigned char *spooldir)
{
	register unsigned char *ptr, *ptr2;
	register unsigned char i, j;
	unsigned char mqueue[MAXPATH], buffer[BUFSIZ];
	struct ffblk ffblk;
	struct wrk *p;
	FILE *fp;

	sprintf(mqueue, "%s/mqueue/*.wrk", spooldir);
	wrktop = p = NULL;
	for(i = 0; ; i = 1) {
		if (i) {
			if (0 != findnext(&ffblk)) break;
		} else {
			if (0 != findfirst(mqueue, &ffblk, FA_RDONLY | FA_ARCH)) break;
		}
		sprintf(mqueue, "%s/mqueue/%s", spooldir, ffblk.ff_name);
		if ((FILE *)NULL == (fp = fopen(mqueue, "rt"))) continue;
		fgets(buffer, BUFSIZ, fp);
		fgets(buffer, BUFSIZ, fp);
		fgets(buffer, BUFSIZ, fp);
		fclose(fp);
		ptr = strtok(buffer, "@\n");
		ptr2 = strtok(NULL, "\n \t");
		j = strcspn(ptr2, ".");
		for (p = wrktop; p; p = p->next) {
			unsigned char i;
			i = strlen(p->host);
			j = (j > i) ? j : i;
			if (!strcmp(p->user, ptr) && !strncmp(p->host, ptr2, j)) break;
		}
		j = strcspn(ptr2, ".");
		if (!p) { /* same user at same host is NOT in list */
			if (NULL == (p = (struct wrk *)malloc(sizeof(struct wrk)))) {
				fputs("Not enough memory.\a\n", stderr);
				exit(1);
			} else {
				if (NULL == (p->user = strdup(ptr))) {
					fputs("Not enough memory.\a\n", stderr);
					exit(1);
				} else {
					if (NULL == (p->host = (char *)malloc(j + 1))) {
						fputs("Not enough memory.\a\n", stderr);
						exit(1);
					} else {
						*p->host = NULL;
						strncat(p->host, ptr2, j);
						p->n = 1;
						p->next = wrktop;
						wrktop = p;
					}
				}
			}
		} else {
			p->n++;
		}
	}
#ifdef DEBUG
	for(p = wrktop; p; p = p->next) printf("%s@%s is %u\n", p->user, p->host, p->n);
	exit(0);
#endif	
}

struct mailq *mailqtop;

/* read status of mailq to memory */
void
readmailq(unsigned char *spooldir)
{
	register unsigned char *ptr, *ptr2, *ptr3, *ptr4;
	register unsigned char i;
	unsigned char buf[BUFSIZ], mqueue[MAXPATH], buffer[BUFSIZ];
	struct ffblk ffblk;
	int number;
	struct stat statbuf;
	struct mailq *p;
	FILE *fp;

	sprintf(mqueue, "%s/mqueue/*.wrk", spooldir);
	mailqtop = p = NULL;
	for(i = 0; ; i = 1) {
		if (i) {
			if (0 != findnext(&ffblk)) break;
		} else {
			if (0 != findfirst(mqueue, &ffblk, FA_RDONLY | FA_ARCH)) break;
		}
		sprintf(mqueue, "%s/mqueue/%s", spooldir, ffblk.ff_name);
		if ((FILE *)NULL == (fp = fopen(mqueue, "rt"))) continue;
		fgets(buffer, BUFSIZ, fp);
		fgets(buffer, BUFSIZ, fp);
		ptr3 = strdup(strtok(buffer, "@\n"));
		ptr4 = strdup(strtok(NULL, "\n \t"));
		if (!ptr3 || !ptr4) {
			fputs("Not Enough Memory.\a\n", stderr);
			exit(1);
		}
		fgets(buffer, BUFSIZ, fp);
		fclose(fp);
		ptr = strtok(buffer, "@\n");
		ptr2 = strtok(NULL, "\n \t");

		/* get file status of real mail */
		number = atoi(ffblk.ff_name);
		sprintf(buf, "%s/mqueue/%d.txt", spooldir, number);
		stat(buf, &statbuf);

		/* add linked list */
		if (NULL == (p = (struct mailq *)malloc(sizeof(struct mailq)))) {
			fputs("Not enough memory.\a\n", stderr);
			exit(1);
		} else {
			if (NULL == (p->user = strdup(ptr))) {
				fputs("Not enough memory.\a\n", stderr);
				exit(1);
			} else {
				if (NULL == (p->host = strdup(ptr2))) {
					fputs("Not enough memory.\a\n", stderr);
					exit(1);
				} else {
					p->bytes = statbuf.st_size;
					p->time = statbuf.st_atime;
					p->fromuser = ptr3;
					p->fromhost = ptr4;
					p->next = mailqtop;
					mailqtop = p;
				}
			}
		}
	}
#ifdef DEBUG
	for(p = mailqtop; p; p = p->next) {
		printf("%s@%s\n", p->user, p->host);
		printf("From %s@%s", p->fromuser, p->fromhost);
		putchar('\n');
	}
	exit(0);
#endif	
}


/* reformnpath() any type path of NEWS to be complete full path */
void
reformnpath(unsigned char *src, const unsigned char *newsdir)
{
	register unsigned char i, j;
	unsigned char buf1[MAXPATH], buf2[MAXPATH];

	i = 0;
	do {
		j = src[i];
		if (j == '.' || j == '\\') j = '/';
		buf1[i++] = j;
	} while (j);
	i = 0;
	do {
		j = newsdir[i];
		if (j == '\\') j = '/';
		buf2[i++] = j;
	} while (j);
	--i;

	if (strnicmp(buf1, buf2, i)) {
		strcat(buf2, "/");
		strcat(buf2, buf1);
		strcpy(src, buf2);
	} else {
		if (buf1[i] == '/') {
			strcpy(src, buf1);
		} else {
			strcat(buf2, "/");
			strcat(buf2, buf1);
			strcpy(src, buf2);
		}
	}
}


int
isnewspath(unsigned char *src, const unsigned char *newsdir)
{
	register unsigned char i, j;
	unsigned char buf1[MAXPATH], buf2[MAXPATH];

	i = 0;
	do {
		j = src[i];
		if (j == '.' || j == '\\') j = '/';
		buf1[i++] = j;
	} while (j);
	i = 0;
	do {
		j = newsdir[i];
		if (j == '\\') j = '/';
		buf2[i++] = j;
	} while (j);
	--i;

	if (strnicmp(buf1, buf2, i)) {
		return FALSE;
	} else {
		if (buf1[i] == '/') {
			return TRUE;
		} else {
			return FALSE;
		}
	}
}
