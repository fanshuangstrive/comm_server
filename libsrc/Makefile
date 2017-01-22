
#COMMDIR=$(shell pwd)
COMMDIR=..

LINUXCC=gcc
LINUXCFLAG= -g   #-m64 -Wall
CC=$(LINUXCC)
CFLAG=$(LINUXCFLAG)

IFLAG=-I${COMMDIR}/incl  -I${TUXDIR}/include 
LFLAG=-L${COMMDIR}/lib

APPLIB=${COMMDIR}/lib/libapp.a
OBJS=socket.o log.o base.o daemon.o 

TARGETS=${APPLIB}

all:${TARGETS}


${APPLIB}:${OBJS}
	ar ruv $@ $?


.c.o:
	$(CC)   $(IFLAG) $(CFLAG) -c $<

clean:
	@ rm -f ${OBJS} ${APPLIB}

