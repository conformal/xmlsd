# $xmlsd$

PROG=mem
#MAN=mem.8
NOMAN=

SRCS= mem.c
COPT+= -O2
DEBUG+= -ggdb3 
CFLAGS+= -Wall
CFLAGS+= -I../../
LDFLAGS+= -lexpat -L../../../obj/ -lxmlsd

.include <bsd.prog.mk>
