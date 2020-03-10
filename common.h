
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* general type */
#ifndef caddr_t
typedef long	caddr_t;
#endif

/* Convenience macros */
#define Number(var)	((sizeof var)/(sizeof var[0]))
#define NewN(type,n)	((type *)Emalloc(sizeof(type)*(n)))
#define New(type)	NewN(type,1)
#define ExistString(p)	( (p) && *(p) )

extern char comname[];
extern char version[];

/* hostname.c */
char *GetHost();

/* common.c */
char *ReplaceChar(char *buf, char from, char to);
caddr_t Emalloc(unsigned bytes);
char *NewString(char *s);
void SplitString( char *av[], char *s );
char *DeleteLastSlash(char *str);
FILE *FopenToRead( char *cmd, char *filename );
FILE *FopenToWrite( char *cmd, char *filename );
void mkdirrec(char *dir);
FILE *dirfopen(char *filename, char *mode);
int copy_file(FILE *fpdest, FILE *fpsrc);

