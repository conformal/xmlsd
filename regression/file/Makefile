# $xmlsd$

PROG=file
#MAN=file.8
NOMAN=

SRCS= file.c
COPT+= -O2
DEBUG+= -ggdb3 
CFLAGS+= -Wall
CFLAGS+= -I../../
LDFLAGS+= -lexpat -L../../../obj/ -lxmlsd

.include <bsd.prog.mk>
