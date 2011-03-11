# $xmlsd$

LOCALBASE?=/usr/local
LIBDIR=${LOCALBASE}/lib

#WANTLINT=
LIB= xmlsd
SRCS= xmlsd.c
DEBUG+= -ggdb3 
CFLAGS+= -Wall -Werror
LDADD+=-lexpat
#MAN= xmlsd.3
NOMAN=
HDRS= xmlsd.h

afterinstall:
	@cd ${.CURDIR}; for i in ${HDRS}; do \
	cmp -s $$i ${LOCALBASE}/include/$$i || \
	${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) $$i ${DESTDIR}${LOCALBASE}/include; \
	echo ${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) $$i ${DESTDIR}${LOCALBASE}/include; \
	done

.include <bsd.own.mk>
.include <bsd.lib.mk>
