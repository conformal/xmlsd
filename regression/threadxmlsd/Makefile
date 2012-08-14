PROG=threadxmlsd
NOMAN=

.if ${.CURDIR} == ${.OBJDIR}
LDADD+= -L${.CURDIR}/../../
.elif ${.CURDIR}/obj == ${.OBJDIR}
LDADD+= -L${.CURDIR}/../../obj
.else
LDADD+= -L${.OBJDIR}/../../
.endif


SRCS= threadxmlsd.c
COPT+= -O2
DEBUG+= -g
CFLAGS+= -Wall -pthread
CFLAGS+= -I../../
LDFLAGS+= -lexpat -lxmlsd -pthread

.include <bsd.prog.mk>
