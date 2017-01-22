

export TUXCONFIG=/home/fs/tuxedoapp/simpapp/src/tuxconfig
export BDMCONFIG=/home/fs/tuxedoapp/simpapp/src/tuxdom

tmshutdown -s simpserv
buildserver -o ../bin/simpserv -f simpserv.c -s TOUPPER
buildclient -o simpcl -f simpcl.c -w
tmboot -s simpserv
