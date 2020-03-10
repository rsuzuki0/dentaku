PROGRAM	= dentaku.exe
DENQPROG= denq.exe
CC	= tcc
MODEL	= s
CFLAGS	= -k- -a -b- -wamp -wnod -wpro -wrvi -wuse -wsig -d -f- -G -O -N -Z
OBJS	= dentaku.obj wexlib.obj netdef.obj smtphead.obj kanji.obj common.obj \
	mid.obj hash.obj tempfile.obj errors.obj derrors.obj ryujilib.obj
DENQOBJS= denq.obj wexlib.obj netdef.obj ryujilib.obj
MANUAL	= dentaku.man denq.man
ARCHIVE	= dntk102.lzh
SRCARCHIVE = densrc.lzh
FILES	= dentaku.exe denq.exe copyrigh.t readme changes *.man *.ntf \
	$(SRCARCHIVE)

.SUFFIXES : .c .obj .ntf

all : $(PROGRAM) $(DENQPROG)

wexlib.c : ../wexlib.c
	cp $< .
ryujilib.c : ../ryujilib.c
	cp $< .
netdef.c : ../netdef.c
	cp $< .

.ntf.man:
	+ntf -o $@ < $<
	
.c.obj:
	@$(CC) -c -m$(MODEL) $(CFLAGS) $<
	@cf $<

$(PROGRAM) : $(OBJS)
	@$(CC) -e$(PROGRAM) -lx -m$(MODEL) @${$(CFLAGS) $#}

$(DENQPROG) : $(DENQOBJS)
	@$(CC) -e$(DENQPROG) -lx -m$(MODEL) @${$(CFLAGS) $#}

netdef.obj : ../netdef.c ../netdef.h
	@$(CC) -c -m$(MODEL) $(CFLAGS) -d- $<

pack : all man
	-rm $(ARCHIVE) $(SRCARCHIVE)
	-rm wexlib.* ryujilib.* netdef.*
	lha a -i2 -n1 -l1 $(SRCARCHIVE) *.c *.h Makefile
	lha a -i2 -n1 -l1 $(ARCHIVE) $(FILES)

man : $(MANUAL)

clean :
	rm *.obj
	rm $(PROGRAM) $(DENQPROG)

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

common.obj: common.h
denq.obj: wexlib.h netdef.h denq.h
dentaku.obj: wexlib.h netdef.h kanji.h smtphead.h dentaku.h
derrors.obj: errors.h
errors.obj: errors.h dentaku.h
hash.obj: dentaku.h hash.h
mid.obj: dentaku.h hash.h errors.h tempfile.h wexlib.h
netdef.obj: netdef.h
smtphead.obj: common.h smtphead.h
tempfile.obj: dentaku.h errors.h tempfile.h
wexlib.obj: wexlib.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
