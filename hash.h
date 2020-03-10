
#define HASHSIZE 4096

int init_hash(char *datafile, char *hashfile);
void close_hash(void);
int creat_init_hash(void);
int creat_hash(char *datatmp);
void add_hash(char *string);
char *search_hash(char *key, char *buf);

int hash(const char *string);
