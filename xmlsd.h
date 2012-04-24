/*
 * Copyright (c) 2010 Marco Peereboom <marco@peereboom.us>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef XMLSD_H
#define XMLSD_H

#ifdef NEED_LIBCLENS
#include <clens.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <sys/queue.h>

/* versioning */
#define XMLSD_STRINGIFY(x)	#x
#define XMLSD_STR(x)		XMLSD_STRINGIFY(x)
#define XMLSD_VERSION_MAJOR	0
#define XMLSD_VERSION_MINOR	8
#define XMLSD_VERSION_PATCH	0
#define XMLSD_VERSION		XMLSD_STR(XMLSD_VERSION_MAJOR) "." \
				XMLSD_STR(XMLSD_VERSION_MINOR) "." \
				XMLSD_STR(XMLSD_VERSION_PATCH)

const char	*xmlsd_verstring(void);
void		 xmlsd_version(int *major, int *minor, int *patch);

#define XMLSD_ERR_UNKNOWN	(-1)
#define XMLSD_ERR_SUCCES	(0)
#define XMLSD_ERR_PARSER	(1)
#define XMLSD_ERR_RESOURCE	(2)
#define XMLSD_ERR_EXTERNAL	(3)
#define XMLSD_ERR_OVERFLOW	(4)
#define XMLSD_ERR_INTEGRITY	(5)

#define XMLSD_TIMEOUT		(5 * 1000) /* 5 seconds */

/* validation structures */
struct xmlsd_v_attr {
	char			*name;
	int			 flags;
#define XMLSD_V_ATTR_F_REQUIRED	 0x0001
};

struct xmlsd_v_elem {
	char			*element;
	char			*path;
	struct xmlsd_v_attr	*attr; /* array of attributes */
	int			 min_occurs; /* Appear at least min times. */
	int			 max_occurs; /* 0 for unbounded. */
};

struct xmlsd_v_elements {
	char			*name;
	struct xmlsd_v_elem	*cmd;
};

/* regular structures */
struct xmlsd_attribute {
	TAILQ_ENTRY(xmlsd_attribute)	entry;
	char				*name;
	char				*value;
};
TAILQ_HEAD(xmlsd_attribute_list, xmlsd_attribute);

struct xmlsd_element {
	TAILQ_ENTRY(xmlsd_element)	entry;
	struct xmlsd_attribute_list	attr_list;
	struct xmlsd_element		*parent;
	char				*name;
	char				*value;
	int				depth;
};
TAILQ_HEAD(xmlsd_element_list, xmlsd_element);

int			xmlsd_unwind(struct xmlsd_element_list *);
int			xmlsd_parse_fileds(int, struct xmlsd_element_list *);
int			xmlsd_parse_file(FILE *, struct xmlsd_element_list *);
int			xmlsd_parse_mem(char *, size_t,
			    struct xmlsd_element_list *);
int			xmlsd_check_path(struct xmlsd_element *, char *);
int			xmlsd_check_attributes(struct xmlsd_element *,
			    struct xmlsd_v_attr *);
int			xmlsd_validate(struct xmlsd_element_list *,
			    struct xmlsd_v_elements *);
/* not a fully qualified search */
char			*xmlsd_get_value(struct xmlsd_element_list *, char *,
			    struct xmlsd_element **);
char			*xmlsd_get_attr(struct xmlsd_element *, char *);
int			xmlsd_check_boolean(char *, int *);

/* xml generate code */
char *xmlsd_generate(struct xmlsd_element_list *xl, void *(*alloc_fn)(size_t),
    size_t *, int);
struct xmlsd_element *xmlsd_create(struct xmlsd_element_list *, const char *);
int xmlsd_set_attr(struct xmlsd_element *, const char *, const char *);
int xmlsd_set_attr_int32(struct xmlsd_element *, const char *, int32_t);
int xmlsd_set_attr_uint32(struct xmlsd_element *, const char *, uint32_t);
int xmlsd_set_attr_int64(struct xmlsd_element *, const char *, int64_t);
int xmlsd_set_attr_uint64(struct xmlsd_element *, const char *, uint64_t);
int xmlsd_set_attr_x32(struct xmlsd_element *, const char *, uint32_t);
int xmlsd_set_attr_x64(struct xmlsd_element *, const char *, uint64_t);
int xmlsd_set_value(struct xmlsd_element *, const char *);
int xmlsd_set_value_int32(struct xmlsd_element *, int32_t);
int xmlsd_set_value_uint32(struct xmlsd_element *, uint32_t);
int xmlsd_set_value_int64(struct xmlsd_element *, int64_t);
int xmlsd_set_value_uint64(struct xmlsd_element *, uint64_t);
int xmlsd_set_value_x32(struct xmlsd_element *, uint32_t);
int xmlsd_set_value_x64(struct xmlsd_element *, uint64_t);
struct xmlsd_element *xmlsd_add_element(struct xmlsd_element_list *,
    struct xmlsd_element *, const char *);
void xmlsd_remove_element(struct xmlsd_element_list *, struct xmlsd_element *);
void xmlsd_free_element(struct xmlsd_element *);

#endif /* XMLSD_H */
