/* disk error check */
/* derrors.c version 0.00 (c) June, 1992 Ryuji Suzuki. */
/* this function depends on BORLAND's compiler and its library. */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <sys/stat.h>
#include "errors.h"

int
chkdfree(char *path, int limit)
{
	struct stat statbuf;
	struct dfree dtable;

	if (!limit) return 0;
	if (!stat(path, &statbuf)) return -1;

	getdfree(statbuf.st_dev, &dtable);
	if ((long)dtable.df_avail * (long)dtable.df_sclus * (long)dtable.df_bsec / 1024L < limit) {
		errprintf("soroban: Too scanty not used disk area %s.\n", path);
		fputs("ディスク足りないぞ！\n\a\a\a", stderr);
		exit(2);
	}
	return 0;
}
