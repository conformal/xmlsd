
PROG=generate
#MAN=generate.8
NOMAN=

.if ${.OBJDIR} == ${.CURDIR}
LIBDIR=../..
.else
LIBDIR=${.CURDIR}/../../obj
.endif

SRCS= generate.c
COPT+= -O2
COPT=-O0
DEBUG+= -ggdb3 
CFLAGS+= -Wall
CFLAGS+= -I../../
LDFLAGS=-static
LDADD+= -L${LIBDIR} -lxmlsd -lexpat 

.include <bsd.prog.mk>
