
BINDIR=Work:Bin
#CFLAGS=-INDK:include_h -bs
CFLAGS=-INDK:include_h

rawdisk: rawdisk.o
#	ln -g -o $@ rawdisk.o -lc
	ln -o $@ rawdisk.o -lm -lc
clean:
	delete rawdisk rawdisk.o rawdisk.dbg
install: rawdisk
	copy rawdisk $(BINDIR) clone
