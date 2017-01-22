COMMDIR=..

LINUXCC=gcc
LINUXCFLAG= -g   #-m64 -Wall
CC=$(LINUXCC)
CFLAG=$(LINUXCFLAG)

IFLAG=-I${COMMDIR}/incl  -I${TUXDIR}/include 
LFLAG=-L${COMMDIR}/lib -lapp

SVR=${COMMDIR}/bin/comm_server
SVROBJS=comm_server.o comm_func.o 

TARGETS= ${SVR}

all:${TARGETS}

${SVR}:${SVROBJS}
	buildclient -o $@  -f "$(CFLAG) $(SVROBJS) $(LFLAG)" -w


.c.o:
	$(CC)   $(IFLAG) $(CFLAG) -c $<

clean:
	@ rm -f ${SVROBJS} ${SVR}

