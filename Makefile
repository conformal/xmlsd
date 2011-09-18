
SYSTEM != uname -s
.if exists(${.CURDIR}/config/Makefile.$(SYSTEM:L))
.  include "${.CURDIR}/config/Makefile.$(SYSTEM:L)"
.endif

LOCALBASE?=/usr/local
LIBDIR=${LOCALBASE}/lib
INCDIR=${LOCALBASE}/include

#WANTLINT=
LIB= xmlsd
SRCS= xmlsd.c xmlsd_generate.c
HDRS= xmlsd.h
#MAN= xmlsd.3
NOMAN=

DEBUG+= -ggdb3 
CFLAGS+= -Wall -Werror
CFLAGS+= -I${.CURDIR} -I${INCDIR}
LDADD+=-lexpat

afterinstall:
	@cd ${.CURDIR}; for i in ${HDRS}; do \
	cmp -s $$i ${DESTDIR}${LOCALBASE}/include/$$i || \
	${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) $$i ${DESTDIR}${LOCALBASE}/include; \
	echo ${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) $$i ${DESTDIR}${LOCALBASE}/include; \
	done

uninstall:
	@for i in $(HDRS); do \
	echo rm -f ${INCDIR}/$$i; \
	rm -f ${INCDIR}/$$i; \
	done

	@for i in $(_LIBS); do \
	echo rm -f ${LIBDIR}/$$i; \
	rm -f ${LIBDIR}/$$i; \
	done

.include <bsd.own.mk>
.include <bsd.lib.mk>
