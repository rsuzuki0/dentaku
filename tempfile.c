/*
 *	tempfile.c: テンポラリファイル管理手続き
 *	version 2.02 1989/11/25  released on 1989/11/25
 *	programmed by JK1LOT(Dai Yokota)
 *	Copyright 1988,89 (C) by JK1LOT(Dai Yokota)
 *	All rights are reserved
 * ver. 2.01 by JK1LOT 88/12/17 debugmode used.
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>		/* ms-dos */
#include <alloc.h>
#include <dir.h>
#include "dentaku.h"
#include "errors.h"
#include "tempfile.h"

extern char	tmpname[]; /* temp_filename template set at oimo.c*/
extern int debugmode;

struct tf_lists {
	struct tf_lists *next;
	char *fname;
	FILE *fp;
};

static struct tf_lists top_tf = { NULL, NULL, NULL };

/*
 * テンポラリファイルを新たに作ってオープンする。
 *   name = オープンしたファイル名を返すためのポインタ
 *   戻り値 = オープンしたファイルポインタ
 */
FILE *tfcreat(char **name)
{
	struct tf_lists *p;

	if ((p = malloc(sizeof(struct tf_lists))) == NULL) {
		alloc_error("tf_open");
	}
	if ((p->fname = strdup(tmpname)) == NULL) {
		alloc_error("tf_open");
	}
	mktemp(p->fname);
	if (!(p->fp = fopen(p->fname, "w+"))) {
		errprintf("%s(tfcreat()):テンポラリファイルがオープンできません\n",
		  comname);
		exit(1);
	}
	p->next = top_tf.next;
	top_tf.next = p;

	*name = p->fname;
	return (p->fp);
}

/*
 * 以前に作ったテンポラリファイルをオープンする。
 *   name = オープンしたいファイル名
 *   mode = オープンするモード
 *   戻り値 = オープンしたファイルポインタ
 */
FILE *tfopen(char *name, char *mode)
{
	struct tf_lists *p;

	for (p = top_tf.next; p; p = p->next)
		if (!strcmp(p->fname, name)) break;
	if (debugmode) {
		if (!p) {
			errprintf("(tfopen): %s は存在しません。\n", name);
			return (NULL);
		}
	}
	if (p == NULL) return NULL;
	if (p->fp) {
		if (debugmode)
			errprintf("(tfopen): %s は既にオープンされています。\n"
			  ,name);
		fclose(p->fp);
		p->fp = NULL;
	}
	if (!(p->fp = fopen(p->fname, mode))) {
		errprintf("%s(tfopen()):テンポラリファイルがオープンできません。\n",
		  comname);
		exit(1);
	}
	return (p->fp);
}

/* テンポラリファイルをクローズする。（削除はしない）*/
void tfclose(FILE *file)
{
	struct tf_lists *p;

	for (p = top_tf.next; p; p = p->next) {
		if (p->fp == file) {
			fclose(p->fp);
			p->fp = NULL;
			break;
		}
	}
	if (debugmode) {
		if (!p) {
			errprintf("存在しないテンポラリファイルのクローズ命令です。\n");
		}
	}
}

/* テンポラリファイルを削除する。（オープン中ならクローズもする）*/
void tfremove(char *name)
{
	struct tf_lists *p, *q;

	for (p = &top_tf; p->next; p = p->next) {
		if (!strcmp(p->next->fname, name)) {
			q = p->next;
			p->next = q->next;
			if (q->fp) {
				fclose(q->fp);
			}
			remove(q->fname);
			free(q->fname);
			free(q);
			return;
		}
	}
	if (debugmode)
		errprintf("存在しないテンポラリファイルの削除命令です。\n");
}

/*
 * テンポラリファイルを全部クローズして削除する。
 * 削除されたファイルを tf_lists から取り除かないためこのルーチンを
 * 呼んで良いのはプログラムの終了時ただ１回のみである。
 */
void tfcloseall()
{
	struct tf_lists *p;
	int count = 0;
	for (p = top_tf.next; p; p = p->next) {
		if (p->fp)
			fclose(p->fp);
		remove(p->fname);
		count++;
	}
	if (debugmode)
		errprintf("(tfcloseall): %d 個のファイルを削除しました。\n", count);
}
