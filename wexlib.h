/* ���q���R�}���h�p�ėp�֐��W */
/* ���̃��C�u���������q���R�}���h�쐬�Ɏg�p����ۂ̏������� dentaku �� */
/* ����ɏ����܂��B���q���ȊO�Ɏg�p����ۂ͂����k���������B */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef long time_t;

struct wrk {
	struct wrk *next;
	char *user;
	char *host;
	unsigned n;
} ;

struct mailq {
	struct mailq *next;
	char *user;
	char *host;
	char *fromuser;
	char *fromhost;
	long bytes;
	time_t time;
} ;

extern struct wrk *wrktop;
extern struct mailq *mailqtop;

time_t
cvoldtime(unsigned char *, unsigned char *);

void
scanwrkfile(unsigned char *spooldir);

void
readmailq(unsigned char *spooldir);

void
reformnpath(unsigned char *src, const unsigned char *newsdir);

int
isnewspath(unsigned char *src, const unsigned char *newsdir);
