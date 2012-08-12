cmn = SE.c mempool.c parser.c 
DRCZfiles = $(cmn) DRCZ.c main.c
asmfiles  = $(cmn) asm.c

DRCZbin = DRCZ
asmbin = asm
execPath = ../the-thing/

all : $(DRCZbin) $(asmbin)

DRCZ : $(DRCZfiles)
	gcc -o $(DRCZbin) $(DRCZfiles)

asm : $(asmfiles)
	gcc -o $(asmbin) $(asmfiles)

install:
	cp $(DRCZbin) $(execPath)
	cp $(asmbin) $(execPath)

uninstall:
	rm -f $(execPath)/$(DRCZbin)
	rm -f $(execPath)/$(asmbin)

clean: 
	rm -f $(DRCZbin) $(asmbin) *.o *~ 