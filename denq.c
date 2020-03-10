/* DENTAKU forms Terakoya news system with Terakoya soroban. */
/* Many other programs for Terakoya news system are availble. */
/* For example; YOMIKAKI(news reader), souji(remaked poi, expire) ...*/
/* You can get infomations about Terakoya network and news systems */
/* by asking PRUG office. */

/* DENQ shows status of DENTAKU-QUEUE and MAIL-QUEUE. */
/* but it is proper to be called as DEN-KYU which means light, */
/*  needless to say. */

/* (C)May, 1992 Ryuji Suzuki(JF7WEX). */
/*     Degenarated voice is not only for made up name */
/*     but build up newest Terakoya network. */


#ifdef __TURBOC__
unsigned _stklen = 16383;
#include <alloc.h>
#else
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
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
#include "netdef.h"
#include "denq.h"

const unsigned char
	program[] = PROGRAM,
	comname[] = PROGRAM,
	version[] = "1.00",
	copyright[] = "(C)May, 1992 Ryuji SUZUKI(jf7wex).";

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
static struct netdefcb const params[] = {
	{NETDEF_DEFI, "Hostname", &hostname, },
	{NETDEF_DEFI, "Domainname", &domain, },
	{NETDEF_DIRE, "SpoolDir", &spool, },
	{NETDEF_DIRE, "Terakoya.newsDir", &newsdir, },
	{NETDEF_DIRE, "Terakoya.libDir", &libdir, },
	{NETDEF_DIRE, "Terakoya.batchDir", &batchdir, },
	{(char *)NULL, (char *)NULL, (char **)NULL},
};

void
printtime(const time_t times)
{
	char *ptr;

	ptr = ctime(&times);
	printf("%.3s, %.2s %.3s %.8s %.4s", ptr, ptr + 8, ptr + 4, ptr + 11, ptr + 20);
}

static int
isindentakumail(void)
{
	struct mailq *p;

	for (p = mailqtop; p; p = p->next)
		if (!strcmp(p->fromhost, hostname) && !strcmp(p->fromuser, "DENTAKU"))
			break;
	return (p != NULL);		
}

static int
analyze_dentaku_sys(FILE *fp_d, struct dentaku *den)
{
	/* analyzing dentaku file */
	register unsigned char *ptr;
	static unsigned char fname[BUFSIZ];
	static unsigned char interfname[BUFSIZ];
	static unsigned char backfname[BUFSIZ];
	static unsigned char buf[BUFSIZ];

	for (;;) {
		if (NULL == fgets(buf, BUFSIZ, fp_d)) return FALSE;
		if (*buf == '#') continue;
		if (*buf == '\n') continue;
		if (strcntchr(buf, ':') < 4) continue;
		break;
	}
	den->desthost = strtok(buf, ": \t");
	den->destuser = strtok(NULL, ": \t");
	den->fname = strtok(NULL, ": \t");
	den->mode = *(ptr = strtok(NULL, ": \t"));
	if (ptr[1] != (char)NULL)
		den->time = atol(ptr + 1);
	else
		den->time = 0;
	den->size = atol(strtok(NULL, ": \t\n"));
	den->n = atoi(strtok(NULL, ": \t\n"));
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


int
main(int argc, const char *argv[])
{
	char buf[BUFSIZ];
	int i;
	
	{
	/* set params */
		if(-1 == loadnetdef(params)){
			fputs("network.def is not found.", stderr);
			exit(1);
		}
		if (!getenv("TMP")) {
			fputs("Set $TMP.\a\n", stderr);
			exit(1);
		}	
		/* analyzing commandline options */
		if (argc > 1){
			for (i = 1; argv[i] != (char *)NULL; i++) {
				if (*argv[i] == '-') {
					switch (argv[i][1]) {
						case 'v':	/* Version */
							fprintf(stderr, "%s: version %s.\n", program, version);
							fputs(copyright, stderr);
							fputs("\n", stderr);
							exit(2);
						case '?':	/* help */
						case 'h':	/* help */
						default:	/* help */
							fprintf(stderr, "%s: version %s.\n",program, version);
							fputs(copyright, stderr);
							fprintf(stderr, "\nUsage: %s -v(ersion) -h(elp) -? [hostname...]", program, version);
							exit(2);
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
	} /* finishd setting parameters */
	tzset();
	readmailq(spool);
	{
		/* main part */
		register int i;
		register char *ptr;
		int flag;
		FILE *fp_d, *fp_batch;
		char buf[BUFSIZ];
		long sumbytes;
		struct dentaku denfile;
		struct stat statbuf;

		flag = 0;
		sprintf(buf, "%s/sys", libdir);
		if ((fp_d = fopen(buf, "rt")) == (FILE *)NULL) {
			fprintf(stderr, "Can't open sys.\n");
			exit(1);
		}
		printf("& DENQ at %s\n", hostname);
		while (analyze_dentaku_sys(fp_d, &denfile)) {
			if (argc == 1 && denfile.mode == 'c') continue;
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
			if (!access(denfile.interfname, 0)) {
				if (!flag) puts("- DENTAKU QUEUE -");
				flag = 1;
				printf("$ STATUS %s@%s\n", denfile.destuser, denfile.desthost);
				puts("\t interfile exists (abnormal)");
			} else if ((FILE *)NULL != (fp_batch = fopen(denfile.fname, "rt"))) {
				if (!flag) puts("- DENTAKU QUEUE -");
				flag = 1;
				printf("$ STATUS %s@%s\n", denfile.destuser, denfile.desthost);
				fputs("\tLatest Access: ", stdout);
				stat(denfile.fname, &statbuf);
				printtime(statbuf.st_atime);
				putchar('\n');
				if (denfile.mode == 'm' || denfile.mode == 'u' || denfile.mode == 'c') {
					puts("\tMode: News");
					sumbytes = 0;
					while (NULL != fgets(buf, BUFSIZ, fp_batch)) {
						strtok(buf, " \t\n");
						ptr = strtok(NULL, " \t\n");
						if (!ptr || !*ptr) { /* bytes field is not available */
							char buf2[BUFSIZ];

							strcpy(buf2, ptr);
							reformnpath(buf2, newsdir);
							if (!stat(buf2, &statbuf)) continue;
							sumbytes += statbuf.st_size;
						} else {
							sumbytes += atol(ptr);
						}
					}
					printf("\tTotal Size: %ld\n", sumbytes);
				} else {
					if (!denfile.time) puts("\tMode: Ihave");
					else printf("\tMode: Ihave (Delay: %ld sec)\n", denfile.time);
				}
			}
			fclose(fp_batch);
		}
		if (mailqtop && isindentakumail()) {
			struct mailq *saveptr, *p, *psave;
			long total;
			int n;
			time_t lateststamp;

			puts("- MAIL QUEUE -");
			while (mailqtop) {
				saveptr = mailqtop;
				mailqtop = saveptr->next;
				if (strcmp(saveptr->fromuser, "DENTAKU")) continue;
				if (strcmp(saveptr->fromhost, hostname)) continue;
				lateststamp = saveptr->time;
				total = saveptr->bytes;
				n = 1;
				for (p = psave = mailqtop; p; ) {
					if ((strcmp(p->host, saveptr->host))
						|| (strcmp(p->fromhost, hostname))
						|| (strcmp(p->fromuser, "DENTAKU"))
						|| (strcmp(p->user, saveptr->user))) {
						psave = p; p = p->next;
						continue;
					}
					total += p->bytes;
					if (p->time > lateststamp) lateststamp = p->time;
					n++;
					/* lines following this cut cell out from list */
					if (psave != mailqtop) {
						psave->next = p->next;
						p = psave->next;
					} else {
						mailqtop = p->next;
						p = psave = mailqtop;
					}
				}
				printf("$ STATUS %s@%s\n", saveptr->user, saveptr->host);
				fputs("\tLatest Enqueue At: ", stdout);
					printtime(lateststamp);
					putchar('\n');
				printf("\tNumber of News: %d\n", n);
				printf("\tTotal Size: %ld\n", total);
			}
		}
		fputs("& FINISH ", stdout);
		printtime(time(NULL));
		putchar('\n');
		fclose(fp_d);
	}
	return 0;
}
