/*
 *	convinience routines
 */

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <dos.h>

#include "common.h"

/* ReplaceChar: replaces globally, character 'from' in 'buf' to 'to' */

char *
ReplaceChar(char *buf, char from, char to)
{
	register char *p=buf;

	if(p)
		for ( ; *p; p++ )
			if( *p==from )
				*p=to;
	return buf;
}

static void
alloc_error(char *mesg)
{
	fprintf(stderr, "%s(%s()): Memory cannot obtained.\n", comname, mesg);
	exit(1);
}

/* Emalloc: malloc function with error detection */

caddr_t
Emalloc(unsigned len)
{
	caddr_t address;
	if(( address = (caddr_t)malloc(len) )==NULL)
		alloc_error("Emalloc");
	return address;
}

/* NewString: allcates new string with error detection */

char *
NewString(char *s)
{
	char *new_s;

	if(!s)
		return NULL;

	if(( new_s = (char *)malloc(sizeof(char)*(strlen(s)+1)) )==NULL)
		alloc_error("NewString");
	strcpy( new_s, s );
	return new_s;
}

/* SplitString: splits string s to av with NULL-termination */

void
SplitString( char *av[], char *s )
{
	char **avp=av;
	*avp = NewString( strtok(s," \t\r\n") );
	while(*avp++)
		*avp = NewString( strtok(NULL," \t\r\n") );
}

char *
DeleteLastSlash(char *str)
{
	char *p=str;

	if(!p)
		return NULL;
	if(!*p)
		return str;
	while(*p)
		p++;
	p--;
	if( *p=='/' || *p=='\\' )
		*p='\0';
	return str;
}

unsigned int
CountLine( char *buf )
{
	register unsigned int i=0;
	while(*buf)
		if( *buf++ == '\n' )
			i++;
	return i;
}

FILE *
FopenToRead( char *cmd, char *filename )
{
	FILE *fpr;

	if( (fpr=fopen(filename,"r")) == NULL )
		fprintf(stderr, "%s: Can't open to read file %s\n",
			cmd, filename);

	return fpr;
}

FILE *
FopenToWrite( char *cmd, char *filename )
{
	FILE *fpw;

	if( (fpw=fopen(filename,"w")) == NULL )
		fprintf(stderr, "%s: Can't open to write file %s\n",
			cmd, filename);

	return fpw;
}

void
mkdirrec(char *dir)
{
	char dir2[MAXPATH];
	char *p;

	if (access(dir, 0) == 0) return;
	if (mkdir(dir) == -1) {
		strcpy(dir2, dir);
		if ((p=(char *)max(
			(char *)strrchr(dir2, '/'), (char *)strrchr(dir2, '\\')
		)) != NULL)
			*p = '\0';
		mkdirrec(dir2);
		mkdir(dir);
	}
}

FILE *
dirfopen(char *filename, char *mode)
{
	FILE *fp;
	char dir[MAXPATH];
	char *p;

	if( !filename || !*filename )
		return NULL;

	if( (fp = fopen(filename, mode)) != NULL )
		return fp;

	strcpy(dir, filename);
	if ((p = (char *)max(
		(char *)strrchr(dir, '/'), (char *)strrchr(dir, '\\')
		)) != NULL)
		*p = '\0';
	mkdirrec(dir);
	fp = fopen(filename, mode);
	if( !fp )
		fprintf( stderr,"Cannot open to write file %s\n", filename );
	return fp;
}

int copy_file(FILE *fpdest, FILE *fpsrc)
{
	char buf[BUFSIZ];

	while (fgets(buf, BUFSIZ, fpsrc) != NULL) {
		if (fputs(buf, fpdest) == EOF) {
			fprintf( stderr,
		"WARNING: Copying failed, Maybe DISK FULL\n" );
			return 0;
		}
	}
	return 1;
}

int
RunShellCommand( char *cmd )
{
	char cmdout[BUFSIZ], *p = cmdout;
	char switc = (getswitchar() == '/')? '/':'\\';
	char pathc = (switc=='/')? '\\':'/';

	while( *cmd ) {
		if( *cmd==switc ) {
			if( *(cmd+1)==switc )
				*p = *cmd++;
			else
				*p = pathc;
		}
		else
			*p = *cmd;
		p++;
		cmd++;
	}
	*p='\0';
	fprintf( stderr, "%s\n", cmdout );
	return system( cmdout );
}
