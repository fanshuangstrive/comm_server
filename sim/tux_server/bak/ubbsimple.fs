#	(c) 2003 BEA Systems, Inc. All Rights Reserved.
#ident	"@(#) samples/atmi/simpapp/ubbsimple	$Revision: 1.7 $"

#Skeleton UBBCONFIG file for the TUXEDO Simple Application.
#Replace the <bracketed> items with the appropriate values.

*RESOURCES
IPCKEY		123456	

#Example:
#IPCKEY		123456

DOMAINID	simpapp
MASTER		simple
MAXACCESSERS	10
MAXSERVERS	5
MAXSERVICES	10
MODEL		SHM
LDBAL		N

*MACHINES
DEFAULT:
		APPDIR="/home/fs/tuxedoapp/simpapp/src"
		TUXCONFIG="/home/fs/tuxedoapp/simpapp/src/tuxconfig"
		TUXDIR="/home/tuxedo/tuxedo11gR1"
#Example:
#		APPDIR="/home/me/simpapp"
#		TUXCONFIG="/home/me/simpapp/tuxconfig"
#		TUXDIR="/usr/tuxedo"

#<Machine-name>	LMID=simple
"myssh.com"	LMID=simple

#Example:
#beatux		LMID=simple

*GROUPS
GROUP1
	LMID=simple	GRPNO=1	OPENINFO=NONE

*SERVERS
DEFAULT:
		CLOPT="-A"

simpserv	SRVGRP=GROUP1 SRVID=1

*SERVICES
TOUPPER
