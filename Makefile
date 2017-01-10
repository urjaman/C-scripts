CC=gcc-4.1
CFLAGS=-Os -march=i486 -fomit-frame-pointer
STRIP=strip
STRIPFLAGS=--strip-unneeded

all: startup shutdown

startup: startup.c scriptlib.o scriptlib.h
	$(CC) $(CFLAGS) -o startup startup.c scriptlib.o
	$(STRIP) $(STRIPFLAGS) startup

shutdown: shutdown.c scriptlib.o scriptlib.h
	$(CC) $(CFLAGS) -o shutdown shutdown.c scriptlib.o
	$(STRIP) $(STRIPFLAGS) shutdown

scriptlib.o:scriptlib.c
	$(CC) $(CFLAGS) -c -o scriptlib.o scriptlib.c

clean:
	rm -rf scriptlib.o startup shutdown

install: startup shutdown
	cp startup shutdown /bin
