
#tmshutdown -s simpserv
buildserver -o simpserv -f simpserv.c -s TOUPPER
buildclient -o simpcl -f simpcl.c -w
#tmboot -s simpserv
