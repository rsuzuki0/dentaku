/* 寺子屋コマンド用汎用関数集 */
/* このライブラリを寺子屋コマンド作成に使用する際の条件等は dentaku の */
/* それに準じます。寺子屋以外に使用する際はご相談ください。 */

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
