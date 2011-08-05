/* $xmlsd$ */
/*
 * Copyright (c) 2011 Dale Rahn <drahn@dalerahn.com>
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
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "xmlsd.h"

#define NL "\r\n"

char *
xmlsd_generate(struct xmlsd_element_list *xl, void *(*alloc_fn)(size_t),
    size_t *xmlszp, int withhdr)
{
        struct xmlsd_element	*xe, *xe_next;
        struct xmlsd_attribute 	*xa;
	char			**depthname;
	char			*obuf, *buf;
	int			bufsz;
	int			max_depth, i;

	obuf = NULL;
	bufsz = 0;

	max_depth = -1;
	TAILQ_FOREACH(xe, xl, entry) {
		if (max_depth < xe->depth)
			max_depth = xe->depth;
	}
	depthname = calloc(max_depth+1, sizeof *depthname);
	if (depthname == NULL)
		return NULL;

for_real:
	buf = obuf;

	if (withhdr) {
		obuf += snprintf(obuf, buf ? bufsz - (obuf-buf) : 0,
		    "<?xml version=\"1.0\"?>" NL NL);
	}
	TAILQ_FOREACH(xe, xl, entry) {
		depthname[xe->depth] = xe->name;
		obuf += snprintf(obuf, buf ? bufsz - (obuf-buf) : 0,
		    "%*s<%s", xe->depth*2, "", xe->name);
		TAILQ_FOREACH(xa, &xe->attr_list, entry)
			obuf += snprintf(obuf, buf ? bufsz - (obuf-buf) : 0,
			    " %s=\"%s\"", xa->name, xa->value);

		xe_next = TAILQ_NEXT(xe, entry);
		if (xe_next != NULL && xe_next->depth > xe->depth) {
			obuf += snprintf(obuf, buf ? bufsz - (obuf-buf) : 0,
			    ">" NL);
		} else {
			if (xe->value != NULL) 
				obuf += snprintf(obuf, buf ? bufsz - (obuf-buf)
				    : 0, ">%s</%s>" NL, xe->value, xe->name);
			else 
				obuf += snprintf(obuf, buf ? bufsz - (obuf-buf)
				    : 0, "/>" NL);
		}
		if (xe_next == NULL) {
			for (i = xe->depth; i > 0; i--)
				obuf += snprintf(obuf, buf ? bufsz - (obuf-buf)
				    : 0, "%*s</%s>" NL, (i-1)*2, "",
				    depthname[i-1]);
		} else {
			if (xe->depth >= xe_next->depth) {
				for (i = xe->depth; i > xe_next->depth; i--) {
					obuf += snprintf(obuf, buf ?
					    bufsz - (obuf-buf) : 0,
					    "%*s</%s>" NL, (i-1)*2, "",
					    depthname[i-1]);
				}
			}
		}

	}
	if (buf == NULL) {
		bufsz = obuf - buf;
		bufsz += 1; /* include NUL */
		obuf = alloc_fn(bufsz);
		if (obuf != NULL) {
			if (xmlszp != NULL)
				*xmlszp = bufsz;
			goto for_real;
		}
	}
	
	return buf;
}
    

struct xmlsd_element *
xmlsd_create(struct xmlsd_element_list *xl, char *name)
{
	struct xmlsd_element *xe;

        TAILQ_INIT(xl);

        xe = calloc(1, sizeof *xe);
        if (xe == NULL)
		goto fail;
	xe->name = strdup(name);
	if (xe->name == NULL)
		goto fail;
        TAILQ_INIT(&xe->attr_list);

	TAILQ_INSERT_TAIL(xl, xe, entry);
	/* depth is set to 0 as result of the calloc */
	
	return xe;
fail:
	if (xe) {
		if (xe->name)
			free(xe->name);
		if (xe)
			free(xe);
	}
	return NULL;
}

int
xmlsd_set_attr_int32(struct xmlsd_element *xe, char *name, int32_t ival)
{
	char *buf;
	int rv;
	asprintf(&buf, "%d", ival);
	if (buf == NULL)
		return 1;

	rv = xmlsd_set_attr(xe, name, buf);

	if (rv)
		free(buf);
	return rv;
}

int
xmlsd_set_attr_uint32(struct xmlsd_element *xe, char *name, uint32_t ival)
{
	char *buf;
	int rv;
	asprintf(&buf, "%u", ival);
	if (buf == NULL)
		return 1;

	rv = xmlsd_set_attr(xe, name, buf);

	if (rv)
		free(buf);
	return rv;
}

int
xmlsd_set_attr_int64(struct xmlsd_element *xe, char *name, int64_t ival)
{
	char *buf;
	int rv;
	asprintf(&buf, "%" PRId64, ival);
	if (buf == NULL)
		return 1;

	rv = xmlsd_set_attr(xe, name, buf);

	if (rv)
		free(buf);
	return rv;
}

int
xmlsd_set_attr_uint64(struct xmlsd_element *xe, char *name, uint64_t ival)
{
	char *buf;
	int rv;
	asprintf(&buf, "%" PRIu64, ival);
	if (buf == NULL)
		return 1;

	rv = xmlsd_set_attr(xe, name, buf);

	if (rv)
		free(buf);
	return rv;
}

int
xmlsd_set_attr_x32(struct xmlsd_element *xe, char *name, uint32_t ival)
{
	char *buf;
	int rv;
	asprintf(&buf, "0x%x", ival);
	if (buf == NULL)
		return 1;

	rv = xmlsd_set_attr(xe, name, buf);

	if (rv)
		free(buf);
	return rv;
}

int
xmlsd_set_attr_x64(struct xmlsd_element *xe, char *name, uint64_t ival)
{
	char *buf;
	int rv;
	asprintf(&buf, "0x%" PRIx64, ival);
	if (buf == NULL)
		return 1;

	rv = xmlsd_set_attr(xe, name, buf);

	if (rv)
		free(buf);
	return rv;
}

int
xmlsd_set_attr(struct xmlsd_element *xe, char *name, char *value)
{
	struct xmlsd_attribute *xa;
        xa = calloc(1, sizeof *xe);
        if (xa == NULL)
		goto fail;
	xa->name = strdup(name);
        if (xa->name == NULL)
		goto fail;
	xa->value = strdup(value);
        if (xa->value == NULL)
		goto fail;

	TAILQ_INSERT_TAIL(&xe->attr_list, xa, entry);

	return 0;
fail:
	if (xa) {
		if (xa->name)
			free(xa->name);
		if (xa->value)
			free(xa->value);
	}
	return 1;
}

int
xmlsd_set_value(struct xmlsd_element *xe, char *value)
{
	if (xe->value)
		free(xe->value);
	xe->value = strdup(value);
	if (xe->value == NULL)
		return 1;
	return 0;
}

struct xmlsd_element *
xmld_add_element(struct xmlsd_element_list *xl, struct xmlsd_element *xe,
    char *name)
{
	struct xmlsd_element *nxe, *prev, *next;

        nxe = calloc(1, sizeof *nxe);
        if (nxe == NULL)
		goto fail;
	nxe->name = strdup(name);
	if (nxe->name == NULL)
		goto fail;
        TAILQ_INIT(&nxe->attr_list);
	nxe->depth = xe->depth+1;

	/* figure out where to insert node! */
	
	prev = xe;
	while((next = TAILQ_NEXT(prev, entry))) {
		if (next->depth <= xe->depth)	
			break;
		prev = next;
	}

	TAILQ_INSERT_AFTER(xl, prev, nxe, entry);

	return nxe;
fail:
	if (nxe) {
		if (nxe->name)
			free(nxe->name);
		if (nxe)
			free(nxe);
	}
	return NULL;
}

#if 0
tag = xmld_add_tag(top, "file");

top = xmlsd_create(&xl, "ct_md_open_read");

top = xmlsd_create("ct_md_open_read");
xmlsd_set_attr(top, "version", STR(CT_MD_OPEN_READ_VERSION));
tag = xmld_add_tag(top, "file");
xmlsd_set_attr(tag, "name", string);
xml = xmlsd_generate(top);
#endif
