/* DENTAKU forms Terakoya news system with Terakoya soroban. */
/* Many other programs for Terakoya news system are availble. */
/* For example; YOMIKAKI(news reader), poi(expire) ...*/
/* You can get infomations about Terakoya network and news systems */
/* by asking PRUG office. */

/* (C)April, 1992 Ryuji Suzuki(JF7WEX). */
/*     Degenarated voice is not only for made up name */
/*     but build up newest Terakoya network. */

/*  version 1.00  June 1992 */
/*  version 1.01  June 1992  A bug caused by my carelessness was fixed. */
/*  version 1.02  June 1992  A bug whose symptom is abort */
/*                           when trying to send expired news was fixed. */

#ifdef __TURBOC__
unsigned _stklen = 16383;
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
#include <io.h>
#include "wexlib.h"
#include "ryujilib.h"
#include "netdef.h"
#include "kanji.h"
#include "smtphead.h"
#include "dentaku.h"

enum processstat { successed, failed, breaked };

const unsigned char
	program[] = PROGRAM,
	comname[] = PROGRAM,
	version[] = "1.02",
	copyright[] = "(C)April, 1992 Ryuji SUZUKI(jf7wex).";


const char TEMPFNAME[] = "DNTKXXXXXX";

struct option {
	unsigned verbose;
	unsigned cat;
} flag;

struct dentaku {
	unsigned char *desthost;
	unsigned char *destuser;
	unsigned char *fname;
	unsigned char *interfname;
	unsigned char *backfname;
	unsigned char mode;
	unsigned char n;
	unsigned long time; /* used only ihave mode */
	unsigned long size;
};


static char *hostname, *domain, *spool = "/spool", *batchdir, *newsdir, *libdir;
static char *tmpl, *spooll, *newsl;
static struct netdefcb const params[] = {
	{NETDEF_DEFI, "Hostname", &hostname, },
	{NETDEF_DEFI, "Domainname", &domain, },
	{NETDEF_DIRE, "SpoolDir", &spool, },
	{NETDEF_DIRE, "Terakoya.newsDir", &newsdir, },
	{NETDEF_DIRE, "Terakoya.libDir", &libdir, },
	{NETDEF_DIRE, "Terakoya.batchDir", &batchdir, },
	{NETDEF_DEFI, "Terakoya.tmpLimit", &tmpl, },
	{NETDEF_DEFI, "Terakoya.spoolLimit", &spooll, },
	{NETDEF_DEFI, "Terakoya.newsLimit", &newsl, },
	{(char *)NULL, (char *)NULL, (char **)NULL},
};


char mid_file[MAXPATH], mid_idx[MAXPATH], mid_lck[MAXPATH];
/* these two lines is for tempfile.c */
char tmpname[] = "DNTKXXXXXX";
int debugmode = FALSE;


static int
analyze_dentaku_sys(FILE *fp_d, struct dentaku *den)
{
	/* analyzing dentaku file */
	register unsigned char *ptr;
	/* unsigned long size; */
	static unsigned char fname[BUFSIZ];
	static unsigned char interfname[BUFSIZ];
	static unsigned char backfname[BUFSIZ];
	static unsigned char buf[BUFSIZ];
	/* int i; */

	for (;;) {
		if (NULL == fgets(buf, BUFSIZ, fp_d)) return FALSE;
		if (*buf == '#') continue;
		if (*buf == '\n') continue;
		if (strcntchr(buf, ':') < 4) continue;
		break;
	}
	den->desthost = strtoke(buf, ": \t");
	den->destuser = strtoke(NULL, ": \t");
	den->fname = strtoke(NULL, ": \t");
	den->mode = *(ptr = strtoke(NULL, ": \t"));
	if (ptr[1] != (char)NULL)
		den->time = atol(ptr + 1);
	else
		den->time = 0;
	den->size = atol(strtoke(NULL, ": \t\n"));
	den->n = atoi(strtoke(NULL, ": \t\n"));
	if (den->n == 0) den->n = (unsigned)(-1);

	den->fname[strcspn(den->fname, "./\\\0")] = (char)NULL;
	sprintf(fname, "%s/%s", batchdir, den->fname);
	sprintf(interfname, "%s.int", fname);
	sprintf(backfname, "%s.bak", fname);
	den->fname = fname;
	den->backfname = backfname;
	den->interfname = interfname;
	return TRUE;
}


enum processstat
catnews_m(FILE *fp_batch, struct dentaku *den, char save_buffer[], long *limitsize)
{
	/* mail mode */
	int i = 0;
	register unsigned char inheader;
	register long newssize;
	struct stat statbuf;
	unsigned char news_buffer[BUFSIZ];
	unsigned char news_fname[BUFSIZ];
	unsigned char mqueue_buffer[BUFSIZ];
	FILE *fp_news, *fp_dest;

	while (!feof(fp_batch)) {
		if (NULL == fgets(news_fname, BUFSIZ, fp_batch))
			break;
		strcpy(save_buffer, news_fname);
		strcpy(news_buffer, strtok(news_fname, " \t\n"));
		reformnpath(news_buffer, newsdir);
		if (flag.verbose){
			fprintf(stderr, "dentaku: now try to open NEWS %s.\n", news_buffer);
		}
		if ((FILE *)NULL != (fp_news = fopen(news_buffer, "rt"))) {
			if (!i) {
				NewMqueueEntry(spool, mqueue_buffer, hostname, den->destuser, den->desthost);
				if ((fp_dest = fopen(mqueue_buffer, "w+t")) == (FILE *)NULL) {
					fputs("dentaku: Disk Error.\a\n", stderr);
					exit(1);
				}
				MakeMail(fp_dest, den->destuser, den->desthost, hostname);
			}
			newssize = atol(strtok(NULL, " \t\n"));
			if (newssize == 0) {
				fstat(fileno(fp_news), &statbuf);
				newssize = statbuf.st_size;
			}
			*limitsize -= newssize;
			if (flag.verbose){
				fprintf(stderr, "    %li Bytes\n", *limitsize);
			}
			if ((*limitsize < 0L) && i){
				fclose(fp_news);
				fclose(fp_dest);
				return successed;
			}
			inheader = 1;
			while (NULL != fgets(news_buffer, BUFSIZ, fp_news)){
				/* write news on mail */
				if (*news_buffer == '\n') inheader = 0;
				if (inheader && !strnicmp(news_buffer, "xref:", 5)) continue;
				fputc(NEWS_MPREFIX, fp_dest);
				fputsNJ(news_buffer, fp_dest);
			}
			fputs("\n", fp_dest);
			i++;
			fclose(fp_news);
			*save_buffer = (char)NULL;
		}
	}
	*save_buffer = (char)NULL;
	fclose(fp_dest);
	return successed;
}


enum processstat
catnews_u(FILE *fp_batch, struct dentaku *den, char save_buffer[], long *limitsize)
{
	/* mail mode */
	register unsigned char inheader;
	register long newssize;
	struct stat statbuf;
	unsigned char news_buffer[BUFSIZ];
	unsigned char news_fname[BUFSIZ];
	unsigned char temp_fname[BUFSIZ];
	unsigned char mqueue_buffer[BUFSIZ];
	FILE *fp_news, *fp_dest, *fp_temp;
	int i = 0;

	while (!feof(fp_batch)){
		if (NULL == fgets(news_fname, BUFSIZ, fp_batch))
			break;
		strcpy(save_buffer, news_fname);
		strcpy(news_buffer, strtok(news_fname, " \t\n"));
		reformnpath(news_buffer, newsdir);
		if (flag.verbose){
			fprintf(stderr, "dentaku: now try to open NEWS %s.\n", news_buffer);
		}
		if ((FILE *)NULL != (fp_news = fopen(news_buffer, "rt"))){
			if (!i) {
				NewMqueueEntry(spool, mqueue_buffer, hostname, den->destuser, den->desthost);
				if ((fp_dest = fopen(mqueue_buffer, "w+t")) == (FILE *)NULL) {
					fputs("dentaku: Disk Error.\a\n", stderr);
					exit(1);
				}
				MakeMail(fp_dest, den->destuser, den->desthost, hostname);
			}
			newssize = atol(strtok(NULL, " \t\n"));
			if (newssize == 0) {
				fstat(fileno(fp_news), &statbuf);
				newssize = statbuf.st_size;
			}
			*limitsize -= newssize;
			if (flag.verbose){
				fprintf(stderr, "    %li Bytes\n", *limitsize);
			}
			if ((*limitsize < 0L) && i){
				fclose(fp_news);
				fclose(fp_dest);
				return successed;
			}
			{ /* making temp file */
				char *ptrtmp;
				strcpy(temp_fname, ptrtmp = getenv("TMP"));
				if (ptrtmp && (temp_fname[strlen(temp_fname) - 1] == '\\' || temp_fname[strlen(temp_fname) -1] == '/')) {
					temp_fname[strlen(temp_fname) - 1] = '\0';
				} else if (!ptrtmp) *temp_fname = '0';
				strcat(temp_fname, "/");
				strcat(temp_fname, TEMPFNAME);
				mktemp(temp_fname);
				if ((FILE *)NULL == (fp_temp = fopen(temp_fname, "w+b"))){
					fputs("dentaku: Disk Full (Can't make file under $TMP)", stderr);
					exit(1);
				}
			}		
			inheader = 1;
			/* copy newsfile to tempfile with coverting kanji code */
			while (NULL != fgets(news_buffer, BUFSIZ, fp_news)) {
				if (*news_buffer == '\n') inheader = 0;
				if (inheader && !strnicmp(news_buffer, "xref:", 5)) continue;
				fputsNJ(news_buffer, fp_temp);
			}

			fclose(fp_news);
			rewind(fp_temp);
			fstat(fileno(fp_temp), &statbuf);
			fprintf(fp_dest, "%c%s %li\n", NEWS_MPREFIX, NEWS_UPREFIX, statbuf.st_size);
			while (NULL != fgetsb(news_buffer, BUFSIZ, fp_temp)){
				/* write news on mail */
				fputc(NEWS_MPREFIX, fp_dest);
				fputs(news_buffer, fp_dest);
			}
			i++;
			fclose(fp_temp);
			remove(temp_fname);
			*save_buffer = (char)NULL;
		}
	}
	*save_buffer = (char)NULL;
	fclose(fp_dest);
	return successed;
}


enum processstat
catnews_stdout(FILE *fp_batch, char save_buffer[], long *limitsize)
{
	/* mail mode */
	long newssize;
	struct stat statbuf;
	unsigned char news_fname[BUFSIZ];
	unsigned char news_buffer[BUFSIZ];
	FILE *fp_news;
	int i = 0;

	while (!feof(fp_batch)) {
		if (NULL == fgets(news_fname, BUFSIZ, fp_batch))
			break;
		strcpy(save_buffer, news_fname);
		strcpy(news_buffer, strtok(news_fname, " \t\n"));
		reformnpath(news_buffer, newsdir);
		if (flag.verbose) {
			fprintf(stderr, "dentaku: now try to open NEWS %s.\n", news_buffer);
		}
		if ((FILE *)NULL != (fp_news = fopen(news_buffer, "rt"))){
			newssize = atol(strtok(NULL, " \t\n"));
			if (newssize == 0) {
				fstat(fileno(fp_news), &statbuf);
				newssize = statbuf.st_size;
			}
			*limitsize -= newssize;
			if (flag.verbose) {
				fprintf(stderr, "    %li Bytes\n", *limitsize);
			}
			if ((*limitsize < 0L) && i) {
				fclose(fp_news);
				return successed;
			}
			while (NULL != fgets(news_buffer, BUFSIZ, fp_news)){
				/* write news on mail */
				fputs(news_buffer, stdout);
			}
			fputc('\n', stdout);
			i++;
			fclose(fp_news);
			*save_buffer = (char)NULL;
		} else {
			fprintf(stderr, "Warning: %s is already erased.\a\n", news_buffer);
		}
	}
	*save_buffer = (char)NULL;
	return successed;
}


enum processstat
make_ihave_news_m(FILE *fp_batch, struct dentaku *den, char save_buffer[], long *limitsize, long dtime)
{
	/* ihave mode */
	long processedtime;
	struct stat statbuf;
	unsigned char news_buffer[BUFSIZ];
	unsigned char news_fname[BUFSIZ];
	unsigned char mqueue_buffer[BUFSIZ];
	FILE *fp_dest;
	int i = 0;

	while (!feof(fp_batch)) {
		if (NULL == fgets(news_fname, BUFSIZ, fp_batch))
			break;
		strcpy(save_buffer, news_fname);
		strcpy(news_buffer, strtok(news_fname, " \t\n"));
		if ((*news_buffer != '<') || (news_buffer[strlen(news_buffer) - 1] != '>')){
			fputs("dentaku: Error Ihave batchfile.\n", stderr);
			fputs("Check that option of sys is 'M'.\n", stderr);
			fclose(fp_dest);
			return failed;
		}
		processedtime = atol(strtok(NULL, " \t\n"));
		if (processedtime <= 0L) {
			processedtime = midtime(news_buffer);
			if (flag.verbose) {
				fprintf(stderr, "time field is not available: %ld %s\n", processedtime, news_buffer);
			}
		}
		if (dtime + processedtime > time(NULL)) {
			if (i) {
				break;
			} else {
				return breaked;
			}
		}
		if (!i) {
			unsigned char seqfile[BUFSIZ];
			NewMqueueEntry(spool, mqueue_buffer, hostname, den->destuser, den->desthost);
			if ((fp_dest = fopen(mqueue_buffer, "w+t")) == (FILE *)NULL) {
				fputs("dentaku: Disk Error.\a\n", stderr);
				exit(1);
			}
			/* making smtp mail header and Ihave news header */
			sprintf(seqfile, "%s/seq", libdir);
			MakeMail(fp_dest, den->destuser, den->desthost, hostname);
			MakeHeaderIhave_m(fp_dest, den->desthost, hostname, seqfile);
			fstat(fileno(fp_dest), &statbuf);
			*limitsize -= statbuf.st_size;
		}
		*limitsize -= strlen(news_buffer) + 3;
		if ((*limitsize < 0L) && i) break;
		fputc(NEWS_MPREFIX, fp_dest);
		fputs(news_buffer, fp_dest);
		fputc('\n', fp_dest);
		i++;
	}
	fclose(fp_dest);
	if (feof(fp_batch)){
		*save_buffer = (char)NULL;
	}
	return successed;
}


enum processstat
make_ihave_news_u(FILE *fp_batch, struct dentaku *den, char save_buffer[], long *limitsize, long dtime)
{
	/* ihave mode */
	long processedtime;
	struct stat statbuf;
	unsigned char news_buffer[BUFSIZ];
	unsigned char news_fname[BUFSIZ];
	unsigned char temp_fname[BUFSIZ];
	unsigned char mqueue_buffer[BUFSIZ];
	FILE *fp_dest, *fp_temp;
	int i = 0;

	while (!feof(fp_batch)){
		if (NULL == fgets(news_fname, BUFSIZ, fp_batch)){
			if (i != 0) break;
			*save_buffer = (char)NULL;
			return failed;
		}
		strcpy(save_buffer, news_fname);
		strcpy(news_buffer, strtok(news_fname, " \t\n"));
		if ((*news_buffer != '<') || (news_buffer[strlen(news_buffer) - 1] != '>')){
			fputs("dentaku: Error Ihave batchfile.\n", stderr);
			fputs("Check that option of sys is 'M'.\n", stderr);
			fclose(fp_temp);
			fclose(fp_dest);
			remove(temp_fname);
			return failed;
		}
		processedtime = atol(strtok(NULL, " \t\n"));
		if (processedtime == 0L) {
			processedtime = midtime(news_buffer);
		}
		if ((processedtime + dtime) > time(NULL)) {
			if (i) {
				break;
			} else {
				return breaked;
			}
		}
		if (!i) { /* making smtp mail header */
			unsigned char seqfile[BUFSIZ];
		 	char *ptrtmp;
			NewMqueueEntry(spool, mqueue_buffer, hostname, den->destuser, den->desthost);
			if ((fp_dest = fopen(mqueue_buffer, "w+t")) == (FILE *)NULL) {
				fputs("dentaku: Disk Error.\a\n", stderr);
				exit(1);
			}
		 /* making temp file */
			strcpy(temp_fname, ptrtmp = getenv("TMP"));
			if (ptrtmp && (temp_fname[strlen(temp_fname) - 1] == '\\' || temp_fname[strlen(temp_fname) -1] == '/')){
				temp_fname[strlen(temp_fname) - 1] = '\0';
			}
			strcat(temp_fname, "/");
			strcat(temp_fname, TEMPFNAME);
			mktemp(temp_fname);
			if ((FILE *)NULL == (fp_temp = fopen(temp_fname, "w+b"))){
				fputs("dentaku: Disk Full (Can't make file under $TMP)", stderr);
				exit(1);
			}
			sprintf(seqfile, "%s/seq", libdir);
			MakeMail(fp_dest, den->destuser, den->desthost, hostname);
			MakeHeaderIhave(fp_temp, den->desthost, hostname, seqfile);
			fstat(fileno(fp_temp), &statbuf);
			*limitsize -= statbuf.st_size;
		}
		*limitsize -= strlen(news_buffer) + 2;
		if ((*limitsize < 0L) && i) break;
		fputs(news_buffer, fp_temp);
		fputc('\n', fp_temp);
		i++;
	}
	rewind(fp_temp);
	fstat(fileno(fp_temp), &statbuf);
	fprintf(fp_dest, "%c%s %li\n", NEWS_MPREFIX, NEWS_UPREFIX, statbuf.st_size);
	while (NULL != fgetsb(news_buffer, BUFSIZ, fp_temp)){
		fputc(NEWS_MPREFIX, fp_dest);
		fputs(news_buffer, fp_dest);
	}
	fclose(fp_temp);
	fclose(fp_dest);
	remove(temp_fname);
	if (feof(fp_batch)){
		*save_buffer = (char)NULL;
	}
	return successed;
}


int
main(int argc, const char *argv[])
{
	char buf[BUFSIZ];
	int i;
	
	{
	/* set params */
		if(-1 == loadnetdef(params)){
			fputs("dentaku: network.def not found.", stderr);
			exit(1);
		}
		if (!getenv("TMP")) {
			fputs("Set $TMP.\a\n", stderr);
			exit(1);
		}	
		/* analyzing commandline options */
		flag.verbose = flag.cat = 0;
		if (argc > 1){
			for (i = 1; argv[i] != (char *)NULL; i++){
				if (*argv[i] == '-') {
					switch (argv[i][1]){
						case 'V':	/* Verbose */
							flag.verbose++;
							break;
						case 'c':	/* permit cat mode user */
							flag.cat++;
							break;
						case 'v':	/* Version */
							fprintf(stderr, "%s: version %s.\n", program, version);
							fputs(copyright, stderr);
							fputs("\n", stderr);
							exit(0);
						case '?':	/* help */
						case 'h':	/* help */
						default:	/* help */
							fprintf(stderr, "%s: version %s.\n",program, version);
							fputs(copyright, stderr);
							fprintf(stderr, "\nUsage: %s -V(erbose) -v(ersion) -h(elp) -c(at) -? [hostname...]", program, version);
							exit(0);
							/* break; */
					}
					argc--;
				}
			}
		}
		strcpy(buf, hostname);
		if (domain && *domain){
			strcat(buf, ".");
			strcat(buf, domain);
		}
		hostname = strdup(buf);
		if (hostname == (char *)NULL) {
			fputs("Not enough memory.\a\n", stderr);
			exit(1);
		}
		if(!newsdir || !(*newsdir)) {
			newsdir = (char *)malloc(strlen(spool) + 6);
			if (newsdir == (char *)NULL) {
				fputs("Not enough memory.\a\n", stderr);
				exit(1);
			}
			strcpy(newsdir, spool);
			strcat(newsdir, "/news");
		}
		if(!batchdir || !(*batchdir)) {
			batchdir = (char *)malloc(strlen(spool) + 7);
			if (batchdir == (char *)NULL) {
				fputs("Not enough memory.\a\n", stderr);
				exit(1);
			}
			strcpy(batchdir, spool);
			strcat(batchdir, "/batch");
		}

		if(!libdir || !(*libdir)) {
			libdir = (char *)malloc(strlen(newsdir) + 5);
			if (libdir == (char *)NULL) {
				fputs("Not enough memory.\a\n", stderr);
				exit(1);
			}
			strcpy(libdir, spool);
			strcat(libdir, "/lib");
		}
		sprintf(mid_file, "%s/%s", libdir, MID_FILE);
		sprintf(mid_idx, "%s/%s", libdir, MID_IDX);
		sprintf(mid_lck, "%s/%s", libdir, MID_LCK);

		chkdfree(getenv("TMP"), atoi(tmpl));
		chkdfree(spool, atoi(spooll));
		chkdfree(newsdir, atoi(newsl));
	} /* finishd setting parameters */
	init_mid(NORMAL);
	tzset();
	{
		/* main part */
		register int i, j;
		/* unsigned char den_buffer[BUFSIZ]; */
		/* unsigned char save_buffer[BUFSIZ]; */
		FILE *fp_d, /* *fp_news, */ *fp_batch, *fp_batchsave;
		char /* *fname, */ buf[BUFSIZ];
		enum processstat ps;
		struct wrk *p;
		struct dentaku denfile;

		if (!flag.cat) scanwrkfile(spool);
		sprintf(buf, "%s/sys", libdir);
		if (flag.verbose) fprintf(stderr, "sysfile: %s\n", buf);
		if ((fp_d = fopen(buf, "rt")) == (FILE *)NULL) {
			fprintf(stderr, "dentaku: Can't open sys.\n");
			exit(1);
		}

		while (analyze_dentaku_sys(fp_d, &denfile)) {
			unsigned char savebuf[BUFSIZ];

			if (denfile.mode == 'c' && !flag.cat) continue;
			if (denfile.mode != 'c' && flag.cat) continue;
			if (argc > 1) {
				for (i = 1; argv[i] != (char *)NULL; i++) {
					int d,a;
					a = strcspn(argv[i], ".");
					d = strcspn(denfile.desthost, ".");
					if (a != d) continue;
					if (!strncmp(argv[i], denfile.desthost, ((a > d) ? a : d)))
						break;
				}
				if (argv[i] == (char *)NULL) continue;
			}
			if (!flag.cat) {
				i = strcspn(denfile.desthost, ".");
				for (p = wrktop; p; p = p->next) {
					if (!strncmp(p->host, denfile.desthost, i) && !strcmp(p->user, denfile.destuser)) break;
				}
				if (p && p->n >= denfile.n) {
					if (flag.verbose) fprintf(stderr, "Too many mails exist for %s@%s\n", denfile.destuser, denfile.desthost);
					continue;
				}
			}
			if (!access(denfile.interfname, 0)) {
				fprintf(stderr, "dentaku: %s exists (abnormal)\n", denfile.interfname);
				continue;
			}
			rename(denfile.fname, denfile.interfname);
			if ((FILE *)NULL == (fp_batch = fopen(denfile.interfname, "rt"))) {
				if (flag.verbose) {
					fprintf(stderr, "No news for %s@%s.\n", denfile.destuser, denfile.desthost);
				}
				continue;
			}
			if (flag.verbose) {
				fprintf(stderr, "To: %s@%s\n", denfile.destuser, denfile.desthost);
			}
			*savebuf = '\0';
			/* analyzing batch file */
			switch (denfile.mode) {
				case 'u':
					ps = catnews_u(fp_batch, &denfile, savebuf, (signed long*)&denfile.size);
					break;
				case 'm':
					ps = catnews_m(fp_batch, &denfile, savebuf, (signed long *)&denfile.size);
					break;
				case 'i':
					ps = make_ihave_news_m(fp_batch, &denfile, savebuf, (signed long *)&denfile.size, denfile.time);
					break;	
				case 'U':
					ps = make_ihave_news_u(fp_batch, &denfile, savebuf, (signed long *)&denfile.size, denfile.time);
					break;	
				case 'c':
					ps = catnews_stdout(fp_batch, savebuf, (signed long *)&denfile.size);
					break;
				default: break;
			}
			if (ps == failed) {
				fclose(fp_batch);
				fputs("failed fgets()", stderr);
				continue;
			} else if (ps == breaked) {
				fclose(fp_batch);
				rename(denfile.interfname, denfile.fname);
				continue;
			}
			j = strcspn(denfile.desthost, ".");
			for (p = wrktop; p; p = p->next) {
				i = strlen(p->host);
				j = (j > i) ? j : i;
				if (!strcmp(p->user, denfile.destuser) && !strncmp(p->host, denfile.desthost, j)) break;
			}
			j = strcspn(denfile.desthost, ".");
			if (!p) { /* same user at same host is NOT in list */
				if (NULL == (p = (struct wrk *)malloc(sizeof(struct wrk)))) {
					fputs("Not enough memory.\a\n", stderr);
					exit(1);
				} else {
					if (NULL == (p->user = strdup(denfile.destuser))) {
						fputs("Not enough memory.\a\n", stderr);
						exit(1);
					} else {
						if (NULL == (p->host = (char *)malloc(j + 1))) {
							fputs("Not enough memory.\a\n", stderr);
							exit(1);
						} else {
							*p->host = NULL;
							strncat(p->host, denfile.desthost, j);
							p->n = 1;
							p->next = wrktop;
							wrktop = p;
						}
					}
				}
			} else {
				p->n++;
			}
			/* recording list that is not read */
			if (*savebuf != (char)NULL) {
				if ((FILE *)NULL == (fp_batchsave = fopen(denfile.fname, "wt"))){
					fputs("dentaku: disk error.\a", stderr);
					exit(1);
				}
				strcat(buf, "\n");
				if (EOF == fputs(savebuf, fp_batchsave)){
					fputs("dentaku: disk error.\a", stderr);
					exit(1);
				}
				while (fgets(buf, BUFSIZ, fp_batch)){
					if (EOF == fputs(buf, fp_batchsave)){
						fputs("dentaku: disk error.\a", stderr);
						exit(1);
					}
				}
				fclose(fp_batchsave);
			}
			fclose(fp_batch);
			remove(denfile.backfname);
			rename(denfile.interfname, denfile.backfname);
		}
	}
	close_mid();
	return 0;
}
