
# Attempt to include platform specific makefile.
# OSNAME may be passed in.
OSNAME ?= $(shell uname -s)
OSNAME := $(shell echo $(OSNAME) | tr A-Z a-z)
-include config/Makefile.$(OSNAME)

# Default paths.
DESTDIR ?=
LOCALBASE ?= /usr/local
BINDIR ?= ${LOCALBASE}/bin
LIBDIR ?= ${LOCALBASE}/lib
INCDIR ?= ${LOCALBASE}/include
MANDIR ?= $(LOCALBASE)/share/man

BUILDVERSION=$(shell sh ${CURDIR}/../buildver.sh)
ifneq ("${BUILDVERSION}", "")
CPPFLAGS+= -DBUILDSTR=\"$(BUILDVERSION)\"
endif

# Use obj directory if it exists.
OBJPREFIX ?= obj/
ifeq "$(wildcard $(OBJPREFIX))" ""
	OBJPREFIX =
endif

# Get shared library version.
-include shlib_version
SO_MAJOR = $(major)
SO_MINOR = $(minor)

# System utils.
AR ?= ar
CC ?= gcc
INSTALL ?= install
LN ?= ln
LNFORCE ?= -f
LNFLAGS ?= -sf
MKDIR ?= mkdir
RM ?= rm -f

# Compiler and linker flags.
CPPFLAGS += -DNEED_LIBCLENS
INCFLAGS += -I$(INCDIR)/clens
WARNFLAGS ?= -Wall -Werror
DEBUG += -g
CFLAGS += $(INCFLAGS) $(WARNFLAGS) $(DEBUG) -O2
LDFLAGS +=
SHARED_OBJ_EXT ?= o

LIB.NAME = xmlsd
LIB.SRCS = xmlsd.c xmlsd_generate.c
LIB.HEADERS = xmlsd.h
LIB.MANPAGES = xmlsd.3
LIB.MLINKS  =xmlsd.3 xmlsd_add_element.3
LIB.MLINKS +=xmlsd.3 xmlsd_check_attributes.3
LIB.MLINKS +=xmlsd.3 xmlsd_check_boolean.3
LIB.MLINKS +=xmlsd.3 xmlsd_check_path.3
LIB.MLINKS +=xmlsd.3 xmlsd_create.3
LIB.MLINKS +=xmlsd.3 xmlsd_free_element.3
LIB.MLINKS +=xmlsd.3 xmlsd_generate.3
LIB.MLINKS +=xmlsd.3 xmlsd_get_attr.3
LIB.MLINKS +=xmlsd.3 xmlsd_get_value.3
LIB.MLINKS +=xmlsd.3 xmlsd_parse_file.3
LIB.MLINKS +=xmlsd.3 xmlsd_parse_fileds.3
LIB.MLINKS +=xmlsd.3 xmlsd_parse_mem.3
LIB.MLINKS +=xmlsd.3 xmlsd_remove_element.3
LIB.MLINKS +=xmlsd.3 xmlsd_set_attr.3
LIB.MLINKS +=xmlsd.3 xmlsd_set_attr_int32.3
LIB.MLINKS +=xmlsd.3 xmlsd_set_attr_int64.3
LIB.MLINKS +=xmlsd.3 xmlsd_set_attr_uint32.3
LIB.MLINKS +=xmlsd.3 xmlsd_set_attr_uint64.3
LIB.MLINKS +=xmlsd.3 xmlsd_set_attr_x32.3
LIB.MLINKS +=xmlsd.3 xmlsd_set_attr_x64.3
LIB.MLINKS +=xmlsd.3 xmlsd_set_value.3
LIB.MLINKS +=xmlsd.3 xmlsd_unwind.3
LIB.MLINKS +=xmlsd.3 xmlsd_validate.3
LIB.MLINKS +=xmlsd.3 xmlsd_version.3
LIB.OBJS = $(addprefix $(OBJPREFIX), $(LIB.SRCS:.c=.o))
LIB.SOBJS = $(addprefix $(OBJPREFIX), $(LIB.SRCS:.c=.$(SHARED_OBJ_EXT)))
LIB.DEPS = $(addsuffix .depend, $(LIB.OBJS))
ifneq "$(LIB.OBJS)" "$(LIB.SOBJS)"
	LIB.DEPS += $(addsuffix .depend, $(LIB.SOBJS))
endif
LIB.MDIRS = $(foreach page, $(LIB.MANPAGES), $(subst ., man, $(suffix $(page))))
LIB.MLINKS := $(foreach page, $(LIB.MLINKS), $(subst ., man, $(suffix $(page)))/$(page))
LIB.LDFLAGS = $(LDFLAGS.EXTRA) $(LDFLAGS)

all: $(OBJPREFIX)$(LIB.SHARED) $(OBJPREFIX)$(LIB.STATIC)

obj:
	-$(MKDIR) obj

$(OBJPREFIX)$(LIB.SHARED): $(LIB.SOBJS)
	$(CC) $(LDFLAGS.SO) $^ $(LIB.LDFLAGS) -o $@

$(OBJPREFIX)$(LIB.STATIC): $(LIB.OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJPREFIX)%.$(SHARED_OBJ_EXT): %.c
	@echo "Generating $@.depend"
	@$(CC) $(INCFLAGS) -MM $(CPPFLAGS) $< | \
	sed 's,$*\.o[ :]*,$@ $@.depend : ,g' > $@.depend
	$(CC) $(CFLAGS) $(PICFLAG) $(CPPFLAGS) -o $@ -c $<

$(OBJPREFIX)%.o: %.c
	@echo "Generating $@.depend"
	@$(CC) $(INCFLAGS) -MM $(CPPFLAGS) $< | \
	sed 's,$*\.o[ :]*,$@ $@.depend : ,g' >> $@.depend
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $< 

depend: 
	@echo "Dependencies are automatically generated.  This target is not necessary."

install:
	$(INSTALL) -m 0755 -d $(DESTDIR)$(LIBDIR)/
	$(INSTALL) -m 0644 $(OBJPREFIX)$(LIB.SHARED) $(DESTDIR)$(LIBDIR)/
	$(LN) $(LNFLAGS) $(LIB.SHARED) $(DESTDIR)$(LIBDIR)/$(LIB.SONAME)
	$(LN) $(LNFLAGS) $(LIB.SHARED) $(DESTDIR)$(LIBDIR)/$(LIB.DEVLNK)
	$(INSTALL) -m 0644 $(OBJPREFIX)$(LIB.STATIC) $(DESTDIR)$(LIBDIR)/
	$(INSTALL) -m 0755 -d $(DESTDIR)$(INCDIR)/
	$(INSTALL) -m 0644 $(LIB.HEADERS) $(DESTDIR)$(INCDIR)/
	$(INSTALL) -m 0755 -d $(addprefix $(DESTDIR)$(MANDIR)/, $(LIB.MDIRS))
	$(foreach page, $(LIB.MANPAGES), \
		$(INSTALL) -m 0444 $(page) $(addprefix $(DESTDIR)$(MANDIR)/, \
		$(subst ., man, $(suffix $(page))))/; \
	)
	@set $(addprefix $(DESTDIR)$(MANDIR)/, $(LIB.MLINKS)); \
	while : ; do \
		case $$# in \
			0) break;; \
			1) echo "Warning: Unbalanced MLINK: $$1"; break;; \
		esac; \
		page=$$1; shift; link=$$1; shift; \
		echo $(LN) $(LNFORCE) $$page $$link; \
		$(LN) $(LNFORCE) $$page $$link; \
	done

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/$(LIB.DEVLNK)
	$(RM) $(DESTDIR)$(LIBDIR)/$(LIB.SONAME)
	$(RM) $(DESTDIR)$(LIBDIR)/$(LIB.SHARED)
	$(RM) $(DESTDIR)$(LIBDIR)/$(LIB.STATIC)
	$(RM) $(addprefix $(DESTDIR)$(INCDIR)/, $(LIB.HEADERS))
	@set $(addprefix $(DESTDIR)$(MANDIR)/, $(LIB.MLINKS)); \
	while : ; do \
		case $$# in \
			0) break;; \
			1) echo "Warning: Unbalanced MLINK: $$1"; break;; \
		esac; \
		page=$$1; shift; link=$$1; shift; \
		echo $(RM) $$link; \
		$(RM) $$link; \
	done
	$(foreach page, $(LIB.MANPAGES), \
		$(RM) $(addprefix $(DESTDIR)$(MANDIR)/, \
		$(subst ., man, $(suffix $(page))))/$(page); \
	)

clean:
	$(RM) $(LIB.SOBJS)
	$(RM) $(OBJPREFIX)$(LIB.SHARED)
	$(RM) $(OBJPREFIX)/$(LIB.SONAME)
	$(RM) $(OBJPREFIX)/$(LIB.DEVLNK)
	$(RM) $(LIB.OBJS)
	$(RM) $(OBJPREFIX)$(LIB.STATIC)
	$(RM) $(LIB.DEPS)

-include $(LIB.DEPS)

.PHONY: clean depend install uninstall

