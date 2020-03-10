char *NewMqueueEntry(char *mspool, char *buf, char *fromhost, char *desthost, char *tohost);
char *rfc_asctime( struct tm *lt, int isgmt, int addweek );
void MakeMail(FILE *fpw, char *destuser, char *desthost, char *host/*, char *seqfile*/);
void MakeHeaderIhave(FILE *fpw, const char *desthost, char *hostname, char *seqfile);
void MakeHeaderIhave_m(FILE *fpw, const char *desthost, char *hostname, char *seqfile);
