/*
*	mknews.c:
*	copyright 1989,90,91 (C) by:
*		Shigeki Matsusima, Dai Yokota, Akiyoshi Kamide
*	all rights are reserved
*/

/*
 *	RFC822/RFC1036 Header Handling Functions
 *		by Akiyoshi Kamide, JS1CPW
 */

/*
*	copyright 1989,90,91 (C) by:
*		Shigeki Matsusima, Dai Yokota, Akiyoshi Kamide
*	all rights are reserved
*/


/* YOMIKAKI のパッケージに入っているソースの *.[ch] のうち、 */
/* DENTAKU に必要な部分のみを残し、修正して smtphead.[ch] */
/* としたものです。ご了承ください。 */
/* 著作権は上記の３人に保留されています。*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef __TURBOC__
#include <dir.h>
#else
#include <direct.h>
#endif
#include <process.h>
#include "common.h"
#include "smtphead.h"

extern char *tzname[2]; /* it must be include time.h !
                           but TC2.0 has bug that time.h don't include it */

static char	*month[] = 
	{"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static char	*week[] = 
	{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static char timebuf[64];

/* GetSeq gets the serial number of the news */

static int
GetSeq(char *seqfile)
{
	register FILE *fp;
	char buf[BUFSIZ];
	int seq;

	if ((fp = fopen(seqfile, "r")) == NULL) {
		fprintf(stderr,
			"%s: Warning: Cannot open to read %s, so made sequence No. to zero\n",
			comname, seqfile);
		return 0;
	}
	fgets(buf, BUFSIZ, fp);
	fclose(fp);
	seq = atoi(buf);
	if ((fp = fopen(seqfile, "w")) == NULL) {
		fprintf(stderr,
			"%s: Warning: Cannot countup sequence No.: %s unwritable\n",
			comname, seqfile);
	}
	else {
		fprintf(fp, "%d", seq+1);
		fclose(fp);
	}
	return(seq);
}

static long
GetMailSeq(char *mspool)
{
	register FILE *fp;
	char buf[BUFSIZ];
	long sequence;
	char Fmseq[MAXPATH];

	sprintf( Fmseq, "%s/mqueue/sequence.seq", mspool );
	if ((fp = fopen(Fmseq, "r")) == NULL) {
		fprintf(stderr,
			"%s: Warning: Cannot open to read %s\n",
			 comname, Fmseq);
		exit(1);
	}
	fgets(buf, BUFSIZ, fp);
	sequence = atol(buf);
	if (sequence < 0L || sequence > 99999998L)
		sequence = 0L;
	sequence++;
	freopen(Fmseq, "w", fp);
	fprintf(fp, "%ld", sequence);
	fclose(fp);
	return(sequence);
}

static char *
rfc_asctime( struct tm *lt, int isgmt, int addweek )
{
	sprintf( timebuf, "%s%s%2u %s %2u %02u:%02u:%02u %s",
		addweek? week[lt->tm_wday]:"",
		addweek? ", ":"",
		lt->tm_mday,
		month[lt->tm_mon],
		lt->tm_year,
		lt->tm_hour,
		lt->tm_min,
		lt->tm_sec,
		isgmt ? "GMT": *tzname? *tzname: "JST" );
	return timebuf;
}

static void
MakeWrk(char *mspool, long seq, char *host, char *destuser, char *dest)
{
	char buf[BUFSIZ];
	FILE *fp;

	sprintf(buf, "%s/mqueue/%ld.wrk", mspool, seq);
	if ((fp = fopen(buf, "w")) == NULL) {
		fprintf(stderr, "%s: Cannot open wrk file %s\n", comname, buf);
		exit(1);
	}
	fprintf(fp, "%s\n", dest);
	fprintf(fp, "%s@%s\n", comname, host);
	fprintf(fp, "%s@%s\n", destuser, dest);
	fclose(fp);
}

char *
NewMqueueEntry(char *mspool, char *buf, char *fromhost, char *destuser, char *tohost)
{
	long mseq;
	MakeWrk( mspool, mseq=GetMailSeq(mspool), fromhost, destuser, tohost );
	sprintf( buf, "%s/mqueue/%ld.txt", mspool, mseq );
	return buf;
}

/* 以下の関数群はかなりの部分を切り離したり、手を入れたり、手を抜いたり、 */
/* 名前を変えたり、勝手に作った部分が多く含まれています。 */

void
MakeMail(FILE *fpw, char *destuser, char *desthost, char *host)
{
	register struct tm *lt;
	long clock;

	/* All times this function uses is in GMT */
	/* get current time */
	time(&clock);
	/* lt = usegmt ? gmtime(&clock) : localtime(&clock); */
	lt = gmtime(&clock);

	/* put SMTP header */
	fprintf(fpw, "Date: %s\n", rfc_asctime(lt, 1, 1));
	fprintf(fpw, "Message-ID: <%02d%02d%02d%02d%02d.AA%05d@%s>\n",
		lt->tm_year,
		lt->tm_mon + 1,
		lt->tm_mday,
		lt->tm_hour,
		lt->tm_min,
		getpid(),
		host);
	fprintf(fpw, "From: %s@%s\n", comname, host);
	fprintf(fpw, "To: %s@%s\n", destuser, desthost);
	fputc('\n', fpw);
}

void
MakeHeaderIhave(FILE *fpw, const char *desthost, char *hostname, char *seqfile)
{
	register struct tm *lt;
	register unsigned char *username;
	long clock;
	unsigned char host[MAXPATH];
	unsigned char dest[MAXPATH];

	username = getenv("USER");
	if (username == NULL) username = "postmaster";
	time(&clock);
	lt = gmtime(&clock);
	*host = *dest = (char)NULL;
	strncat(host, hostname, (strchr(hostname, '.') ? strcspn(hostname, ".") : MAXPATH-1));
	strncat(dest, desthost, (strchr(desthost, '.') ? strcspn(desthost, ".") : MAXPATH-1));
/*	strcpy(host, hostname);
	strcpy(dest, desthost);
	fputs(host, stderr);
	fputc('\n', stderr);
	fputs(dest, stderr);
	fputc('\n', stderr); */
	fprintf(fpw, "Path: %s\n", comname);
	fprintf(fpw, "From: %s@%s\n", username, hostname);
	fprintf(fpw, "Newsgroups: to.%s\n", dest);
	fprintf(fpw, "Subject: ihave %s\n", hostname);
	fprintf(fpw, "Control: ihave %s\n", hostname);
/*	fprintf(fpw, "Approved: %s@%s\n", username, hostname); */
	fprintf(fpw, "Sender: %s@%s\n", comname, hostname);
	fprintf(fpw, "Date: %s\n", rfc_asctime(lt, 1, 1) );
	fprintf(fpw, "Message-ID: <%x%x%x.%d@%s>\n", lt->tm_mday,
		lt->tm_mon, lt->tm_hour, GetSeq(seqfile), hostname); 
	fputc('\n', fpw);
}

void
MakeHeaderIhave_m(FILE *fpw, const char *desthost, char *hostname, char *seqfile)
{
	register unsigned char *username;
	register struct tm *lt;
	long clock;
	unsigned char host[MAXPATH];
	unsigned char dest[MAXPATH];

	username = getenv("USER");
	if (username == NULL) username = "postmaster";
	time(&clock);
	lt = gmtime(&clock);
	*host = *dest = (char)NULL;
	strncat(host, hostname, (strchr(hostname, '.') ? strcspn(hostname, ".") : MAXPATH-1));
	strncat(dest, desthost, (strchr(desthost, '.') ? strcspn(desthost, ".") : MAXPATH-1));
/*	strcpy(host, hostname);
	strcpy(dest, desthost); */
/*	fputs(host, stderr);
	fputc('\n', stderr);
	fputs(dest, stderr);
	fputc('\n', stderr); */
	fprintf(fpw, "NPath: %s\n", comname);
	fprintf(fpw, "NFrom: %s@%s\n", username, hostname);
	fprintf(fpw, "NNewsgroups: to.%s\n", dest);
	fprintf(fpw, "NSubject: ihave %s\n", hostname);
	fprintf(fpw, "NControl: ihave %s\n", hostname);
/*	fprintf(fpw, "NApproved: %s@%s\n", username, hostname); */
	fprintf(fpw, "NSender: %s@%s\n", comname, hostname);
	fprintf(fpw, "NDate: %s\n", rfc_asctime(lt, 1, 1) );
	fprintf(fpw, "NMessage-ID: <%x%x%x.%d@%s>\n", lt->tm_mday,
		lt->tm_mon, lt->tm_hour, GetSeq(seqfile), hostname); 
	fputs("N\n", fpw);
}
