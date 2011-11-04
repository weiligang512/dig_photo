CURRENTDIR	:= .

SRCDIR		:= $(CURRENTDIR)/src

INCLUDEDIR	:= $(CURRENTDIR)/include

SRCFILE		:= $(SRCDIR)/%.c

TARGETSOBJ	:= digphoto.o frabuff.o filesrc.o mount.o key.o

CC		:= arm-linux-gcc

CFLAGS		:= -Wall -DDEBUG -I$(INCLUDEDIR) -g

LDFLAGS		:= -static -ljpeg -lpthread 

TARGETS 	:= digphoto

.PHONY: all clean

#all : $(TARGETS)



$(TARGETS) : $(TARGETSOBJ)
	$(CC) -g $^ -o $@ $(LDFLAGS)

%.o : $(SRCFILE)
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm -f *.o $(TARGETS) $(SRCDIR)/*~ $(INCLUDEDIR)/*~ $(CURRENTDIR)/*~
