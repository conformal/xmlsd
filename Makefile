# $xmlsd$

LOCALBASE?=/usr/local
LIBDIR=${LOCALBASE}/lib
INCDIR=${LOCALBASE}/include

#WANTLINT=
LIB= xmlsd
SRCS= xmlsd.c
DEBUG+= -ggdb3 
CFLAGS+= -Wall -Werror
CFLAGS+= -I${.CURDIR} -I${INCDIR}
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

uninstall:
	@for i in $(HDRS); do \
	echo rm -f ${INCDIR}/$$i ;\
	rm -f ${INCDIR}/$$i; \
	done

	@for i in $(_LIBS); do \
	echo rm -f ${LIBDIR}/$$i ;\
	rm -f ${LIBDIR}/$$i; \
	done

.include <bsd.own.mk>
.include <bsd.lib.mk>
