PROG=generate
NOMAN=

.if ${.CURDIR} == ${.OBJDIR}
LDADD+= -L${.CURDIR}/../../
.elif ${.CURDIR}/obj == ${.OBJDIR}
LDADD+= -L${.CURDIR}/../../obj
.else
LDADD+= -L${.OBJDIR}/../../
.endif

SRCS= generate.c
DEBUG+= -g
CFLAGS+= -Wall
CFLAGS+= -I../../
LDFLAGS=-static
LDADD+= -lxmlsd -lexpat 

.include <bsd.prog.mk>
