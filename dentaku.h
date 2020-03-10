#define TRUE 1
#define FALSE 0
#define NEWS_MPREFIX 'N'
#define NEWS_UPREFIX "#! rnews"
#define PROGRAM "DENTAKU"

extern const unsigned char
	program[],
	comname[],
	version[],
	copyright[];

void reformnpath(unsigned char *, const unsigned char *);
long midtime(char *);

/* from soroban.h */

#define TRUE		1
#define FALSE		0

#define NORMAL 0
#define CONFIG 1

#define	MID_FILE	"/history"
#define	MID_IDX		"/history.idx"
#define	MID_LCK		"/history.lck"

int chkdfree(char *path, int limit);

extern char mid_file[];
extern char mid_idx[];
extern char mid_lck[];

void init_mid(int mode);
void close_mid(void);
void config_mid(void);
void add_mid(char *mid, char *filename);
int srch_mid(char *mid);
FILE *midopen(char *mid, char *mode);
int midrm(char *mid);
FILE *midlckopen(char *mid, char *mode);
void midlckrm(char *mid);
long filetimecomp(char *fn1, char *fn2);
