CC = gcc
OBJ = edit.o term.o screen.o stuff.o io.o editor.o

#----------------------------------------------------------
# FreeBSD
#----------------------------------------------------------
#CFLAGS ?= -O -pipe
#CFLAGS += -DVEDITOR
#LIBS = -ltermcap -lcompat

#----------------------------------------------------------
# Linux
#----------------------------------------------------------
CFLAGS = -DVEDITOR -DLINUX
LIBS = -lncurses

#----------------------------------------------------------
# SunOS
#----------------------------------------------------------
#CFLAGS = -DVEDITOR
#LIBS = -ltermcap

all: ve

clean:
	rm -f *.o ve

install: all
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/share/ve
	install ve $(PREFIX)/bin
	install -m 444 ve.hlp $(PREFIX)/share/ve

ve: $(OBJ)
	$(CC) $(CFLAGS) -o ve $(OBJ) $(LIBS)
