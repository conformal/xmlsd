# $xmlsd$

PREFIX?=${DESTDIR}/usr/local
LIBDIR=${PREFIX}/lib

#WANTLINT=
LIB= xmlsd
SRCS= xmlsd.c
DEBUG+= -ggdb3 
CFLAGS+= -Wall -Werror
LDFLAGS+=-lexpat
#MAN= xmlsd.3
NOMAN=
HDRS= xmlsd.h

afterinstall:
	@cd ${.CURDIR}; for i in ${HDRS}; do \
	cmp -s $$i ${PREFIX}/include/$$i || \
	${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) $$i ${PREFIX}/include; \
	echo ${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) $$i ${PREFIX}/include; \
	done

.include <bsd.own.mk>
.include <bsd.lib.mk>
