/* $Header: rjslib.hv  1.1  92/09/17 04:10:42  jf7wex  Exp $ */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


char *
strtoke(char *, const char *);

unsigned
strcntchr(const char *, char);

char *
fgetsb(char buf[], int n, FILE *fp);

char *
getmline(FILE *fp);
