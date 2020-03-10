/*
 *	errors.h: header file for errors.c
 *  version 1.0 1988/12/19 released on 1988/12/19
 *	programmed by JK1LOT (Dai Yokota)
 *	Copyright 1988 (C) by Dai Yokota
 *	All rights are reserved
 *
 *	this header is for ms-dos use only
 *	this header is for Turbo-C use only
 *	this header is for KA9Q tcp-ip package use only
 *
 */

void alloc_error(char *mesg);
void disk_error(char *fn);
void errprintf(char *fmt,...);
