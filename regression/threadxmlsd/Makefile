
PROG=threadxmlsd
#MAN=threadxmlsd.8
NOMAN=

SRCS= threadxmlsd.c
COPT+= -O2
DEBUG+= -g
CFLAGS+= -Wall -pthread
CFLAGS+= -I../../
LDFLAGS+= -lexpat -L../../ -lxmlsd -pthread

.include <bsd.prog.mk>
