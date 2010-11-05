# $xmlsd$

PREFIX?=${DESTDIR}/usr/local
LIBDIR=${PREFIX}/lib

.include <${.CURDIR}/../shlib_version>
.PATH: ${.CURDIR}/..
SHLIB_MAJOR=${major}

#WANTLINT=
LIB= xmlsd
SRCS= xmlsd.c
DEBUG+= -ggdb3 
CFLAGS+= -Wall -Werror
CFLAGS+= -I/usr/X11R6/include -I/usr/local/include
LDADD+=-L/usr/X11R6/lib -L/usr/local/lib -lexpat
#MAN= xmlsd.3
NO_MAN=
HDRS= xmlsd.h

afterinstall:
	@cd ${.CURDIR}; for i in ${HDRS}; do \
	cmp -s ../$$i ${PREFIX}/include/$$i || \
	${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) ../$$i ${PREFIX}/include; \
	echo ${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) ../$$i ${PREFIX}/include; \
	done

.include <bsd.own.mk>
.include <bsd.lib.mk>
