CFLAGS=-DUNIX -lreadline -lcurses 
DEBUG=-g
#DEBUG=

all: shell


shell:	shell.c parse.c parse.h
	gcc --std=gnu99 $(CFLAGS) $(DEBUG) shell.c parse.c -o shell
clean:
	rm -f shell *~