# $xmlsd$

#.PATH:		${.CURDIR}/..

#WANTLINT=
LIB= xmlsd
SRCS= xmlsd.c
DEBUG+= -ggdb3 
CFLAGS+= -Wall -Werror
LDFLAGS+=-lexpat
#MAN= xmlsd.3
NOMAN=
HDRS= xmlsd.h

includes:
	@cd ${.CURDIR}; for i in ${HDRS}; do \
	cmp -s $$i ${DESTDIR}/usr/include/$$i || \
	${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) $$i \
	${DESTDIR}/usr/include; done

.include <bsd.own.mk>
.include <bsd.lib.mk>
