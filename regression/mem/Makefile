PROG=mem
NOMAN=

.if ${.CURDIR} == ${.OBJDIR}
LDADD+= -L${.CURDIR}/../../
.elif ${.CURDIR}/obj == ${.OBJDIR}
LDADD+= -L${.CURDIR}/../../obj
.else
LDADD+= -L${.OBJDIR}/../../
.endif

SRCS= mem.c
COPT+= -O2
DEBUG+= -g
CFLAGS+= -Wall
CFLAGS+= -I../../
LDFLAGS+= -lexpat -lxmlsd

.include <bsd.prog.mk>
