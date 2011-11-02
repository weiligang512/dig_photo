CURRENTDIR	:= .

SRCDIR		:= $(CURRENTDIR)/src

INCLUDEDIR	:= $(CURRENTDIR)/include

SRCFILE		:= $(SRCDIR)/%.c

TARGETSOBJ	:= digphoto.o frabuff.o filesrc.o mount.o key.o

CC		:= gcc

CFLAGS		:= -Wall -I$(INCLUDEDIR) -g

LDFLAGS		:= -ljpeg -lpthread -lcurses

TARGETS 	:= digphoto

.PHONY: all clean

#all : $(TARGETS)



$(TARGETS) : $(TARGETSOBJ)
	$(CC) -g $^ -o $@ $(LDFLAGS)

%.o : $(SRCFILE)
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm -f *.o $(TARGETS) $(SRCDIR)/*~ $(INCLUDEDIR)/*~ $(CURRENTDIR)/*~
