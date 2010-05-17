# $xmlsd$

PROG=regress
#MAN=regress.8
NOMAN=

SRCS= regress.c
COPT+= -O2
DEBUG+= -ggdb3 
CFLAGS+= -Wall
CFLAGS+= -I../../
LDFLAGS+= -lexpat -L../../obj/ -lxmlsd

.include <bsd.prog.mk>
