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
#define XMLSD_VERSION_PATCH	1
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

/* XML attribute */
struct xmlsd_attribute;
const char		*xmlsd_attr_get_name(struct xmlsd_attribute *);
const char		*xmlsd_attr_get_value(struct xmlsd_attribute *);

/*
 * XML element inteface definition
 */
struct xmlsd_element;
/* element get and iteration interface */
const char		*xmlsd_elem_get_name(struct xmlsd_element *);
const char		*xmlsd_elem_get_value(struct xmlsd_element *);
long long 		 xmlsd_elem_get_value_strtonum(struct xmlsd_element *,
			     long long, long long, const char **);
unsigned long long 	 xmlsd_elem_get_value_hexnum(struct xmlsd_element *,
			     unsigned long long, unsigned long long,
			     const char **);
int			 xmlsd_elem_get_value_boolean(struct xmlsd_element *,
			     int *, int);
int			 xmlsd_elem_get_depth(struct xmlsd_element *);
struct xmlsd_element	*xmlsd_elem_get_parent(struct xmlsd_element *);
struct xmlsd_attribute	*xmlsd_elem_find_attr(struct xmlsd_element *,
			     const char *);
struct xmlsd_attribute	*xmlsd_elem_get_first_attr(struct xmlsd_element *);
struct xmlsd_attribute	*xmlsd_elem_get_next_attr(struct xmlsd_element *,
			     struct xmlsd_attribute *);
struct xmlsd_attribute	*xmlsd_elem_get_last_attr(struct xmlsd_element *);
struct xmlsd_attribute	*xmlsd_elem_get_previous_attr(struct xmlsd_element *,
			     struct xmlsd_attribute *);
#define XMLSD_ELEM_FOREACH_ATTR(attr, elem)				\
	for ((attr) = xmlsd_elem_get_first_attr(elem);			\
	    (attr) != NULL; (attr) = xmlsd_elem_get_next_attr(elem, attr))
struct xmlsd_element	*xmlsd_elem_find_child(struct xmlsd_element *,
			     const char *);
struct xmlsd_element	*xmlsd_elem_get_first_child(struct xmlsd_element *);
struct xmlsd_element	*xmlsd_elem_get_next_child(struct xmlsd_element *,
			     struct xmlsd_element *);
struct xmlsd_element	*xmlsd_elem_get_last_child(struct xmlsd_element *);
struct xmlsd_element	*xmlsd_elem_get_previous_child(struct xmlsd_element *,
			     struct xmlsd_element *);
#define XMLSD_ELEM_FOREACH_CHILDREN(child, elem)			\
	for ((child) = xmlsd_elem_get_first_child(elem);		\
	    (child) != NULL; (child) = xmlsd_elem_get_next_child(elem, child))
/* attribute getting  interface */
const char		*xmlsd_elem_get_attr(struct xmlsd_element *,
			     const char *);
long long 		 xmlsd_elem_get_attr_strtonum(struct xmlsd_element *,
			     const char *, long long, long long, const char **);
unsigned long long 	 xmlsd_elem_get_attr_hexnum(struct xmlsd_element *,
			     const char *, unsigned long long,
			     unsigned long long, const char **);
/* XXX real bool? */
int			 xmlsd_elem_get_attr_boolean(struct xmlsd_element *,
			     const char *, int *, int);
/* element setting interface */
int			 xmlsd_elem_set_attr(struct xmlsd_element *, const char *, const char *);
int			 xmlsd_elem_set_attr_int32(struct xmlsd_element *, const char *, int32_t);
int			 xmlsd_elem_set_attr_uint32(struct xmlsd_element *, const char *, uint32_t);
int			 xmlsd_elem_set_attr_int64(struct xmlsd_element *, const char *, int64_t);
int			 xmlsd_elem_set_attr_uint64(struct xmlsd_element *, const char *, uint64_t);
int			 xmlsd_elem_set_attr_x32(struct xmlsd_element *, const char *, uint32_t);
int			 xmlsd_elem_set_attr_x64(struct xmlsd_element *, const char *, uint64_t);
int			 xmlsd_elem_set_value(struct xmlsd_element *, const char *);
int			 xmlsd_elem_set_value_int32(struct xmlsd_element *, int32_t);
int			 xmlsd_elem_set_value_uint32(struct xmlsd_element *, uint32_t);
int			 xmlsd_elem_set_value_int64(struct xmlsd_element *, int64_t);
int			 xmlsd_elem_set_value_uint64(struct xmlsd_element *, uint64_t);
int			 xmlsd_elem_set_value_x32(struct xmlsd_element *, uint32_t);
int			 xmlsd_elem_set_value_x64(struct xmlsd_element *, uint64_t);
void			 xmlsd_elem_free(struct xmlsd_element *);


/* not a fully qualified search */

/* XML document parsing and creation */
struct xmlsd_document;
int			 xmlsd_doc_alloc(struct xmlsd_document **);
void			 xmlsd_doc_clear(struct xmlsd_document *);
void			 xmlsd_doc_free(struct xmlsd_document *);
int			 xmlsd_doc_is_empty(struct xmlsd_document *);
struct xmlsd_element	*xmlsd_doc_get_first_elem(struct xmlsd_document *);
struct xmlsd_element	*xmlsd_doc_get_next_elem(struct xmlsd_document *,
			     struct xmlsd_element *);
struct xmlsd_element	*xmlsd_doc_get_last_elem(struct xmlsd_document *);
struct xmlsd_element	*xmlsd_doc_get_previous_elem(struct xmlsd_document *,
			     struct xmlsd_element *);
/* foreach each top level element in a document */
#define XMLSD_DOC_FOREACH_ELEM(elem, doc)				\
	for ((elem) = xmlsd_doc_get_first_elem(doc); (elem) != NULL;	\
	   (elem) = xmlsd_doc_get_next_elem(doc, elem))
/* searches top level for a named value */
const char		*xmlsd_doc_find_value(struct xmlsd_document *,
			     const char *, struct xmlsd_element **);

int			 xmlsd_parse_fileds(int, struct xmlsd_document *);
int			 xmlsd_parse_file(FILE *, struct xmlsd_document  *);
int			 xmlsd_parse_mem(const char *, size_t,
			    struct xmlsd_document *);

#define XMLSD_GEN_ADD_HEADER	1
char *xmlsd_generate(struct xmlsd_document *xl, void *(*alloc_fn)(size_t),
    size_t *, int);
struct xmlsd_element	*xmlsd_doc_add_elem(struct xmlsd_document *,
			     struct xmlsd_element *, const char *);
void			 xmlsd_doc_remove_elem(struct xmlsd_document *,
			     struct xmlsd_element *);

/* validation */
int			 xmlsd_validate(struct xmlsd_document *,
			    struct xmlsd_v_elements *);

#endif /* XMLSD_H */
