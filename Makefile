
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
MAN= xmlsd.3
MLINKS+=xmlsd.3 xmlsd_add_element.3
MLINKS+=xmlsd.3 xmlsd_check_attributes.3
MLINKS+=xmlsd.3 xmlsd_check_boolean.3
MLINKS+=xmlsd.3 xmlsd_check_path.3
MLINKS+=xmlsd.3 xmlsd_create.3
MLINKS+=xmlsd.3 xmlsd_free_element.3
MLINKS+=xmlsd.3 xmlsd_generate.3
MLINKS+=xmlsd.3 xmlsd_get_attr.3
MLINKS+=xmlsd.3 xmlsd_get_value.3
MLINKS+=xmlsd.3 xmlsd_parse_file.3
MLINKS+=xmlsd.3 xmlsd_parse_fileds.3
MLINKS+=xmlsd.3 xmlsd_parse_mem.3
MLINKS+=xmlsd.3 xmlsd_remove_element.3
MLINKS+=xmlsd.3 xmlsd_set_attr.3
MLINKS+=xmlsd.3 xmlsd_set_attr_int32.3
MLINKS+=xmlsd.3 xmlsd_set_attr_int64.3
MLINKS+=xmlsd.3 xmlsd_set_attr_uint32.3
MLINKS+=xmlsd.3 xmlsd_set_attr_uint64.3
MLINKS+=xmlsd.3 xmlsd_set_attr_x32.3
MLINKS+=xmlsd.3 xmlsd_set_attr_x64.3
MLINKS+=xmlsd.3 xmlsd_set_value.3
MLINKS+=xmlsd.3 xmlsd_unwind.3
MLINKS+=xmlsd.3 xmlsd_validate.3
MLINKS+=xmlsd.3 xmlsd_version.3

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
