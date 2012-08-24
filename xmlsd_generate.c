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
#include "xmlsd_internal.h"

#define NL "\r\n"

/*
 * encode characters that are invalid in xml attributes and values and return
 * a pointer to the end of the string (the NUL terminated part).
 *
 * if dry_run is set don't actually change teh data, just calculate the length.
 */
static char *
encode_data(char *buf, const char *data, int dry_run)
{
	while (*data != '\0') {
		if (*data == '&') {
			if (dry_run != 0) {
				buf[0] = '&';
				buf[1] = 'a';
				buf[2] = 'm';
				buf[3] = 'p';
				buf[4] = ';';
			}
			buf += 5;	/* &amp; */
		} else if (*data == '<') {
			if (dry_run != 0) {
				buf[0] = '&';
				buf[1] = 'l';
				buf[2] = 't';
				buf[3] = ';';
			}

			buf += 4;	/* &lt; */
		} else if (*data == '>') {
			if (dry_run != 0) {
				buf[0] = '&';
				buf[1] = 'g';
				buf[2] = 't';
				buf[3] = ';';
			}
			buf += 4;	/* &gt; */
		} else if (*data == '"') {
			if (dry_run != 0) {
				buf[0] = '&';
				buf[1] = 'q';
				buf[2] = 'u';
				buf[3] = 'o';
				buf[4] = 't';
				buf[5] = ';';
			}
			buf += 6;
		} else {
			/* no encoding, just copy */
			if (dry_run != 0)
				*buf = *data;
			buf++;
		}
		data++;
	}
	/*
	 * Don't forget to NUL terminate, strictly not needed due to context
	 * as this will never be the last thing to touch the buffer.
	 */
	if (dry_run != 0) {
		*buf = '\0';
	}

	return (buf);
}

size_t
xmlsd_generate_elem(struct xmlsd_element *xe, char *buf, size_t bufsz,
    int dry_run)
{
	struct xmlsd_attribute	*xa;
	struct xmlsd_element	*xc;
	char			*obuf = buf;

	obuf += snprintf(obuf, buf ? bufsz - (obuf-buf) : 0,
	    "%*s<%s", xe->depth * 2, "", xe->name);
	XMLSD_ELEM_FOREACH_ATTR(xa, xe) {
		obuf += snprintf(obuf, dry_run ? bufsz - (obuf-buf) : 0,
		    " %s=\"", xa->name);
		obuf = encode_data(obuf, xa->value, dry_run);
		/* it would likely be more efficient to inline this */
		obuf += snprintf(obuf, dry_run ? bufsz - (obuf-buf) : 0,
		    "\"");
	}

	/* should have only one of children or value */
	if (xmlsd_elem_get_first_child(xe) == NULL && xe->value == NULL) {
		obuf += snprintf(obuf, dry_run ? bufsz - (obuf-buf) : 0,
		    "/>" NL);
	} else if (xe->value != NULL) {
		obuf += snprintf(obuf, dry_run ? bufsz - (obuf-buf)
			    : 0, ">");
		obuf = encode_data(obuf, xe->value, dry_run);
		obuf += snprintf(obuf, dry_run ? bufsz - (obuf-buf)
			    : 0, "</%s>" NL, xe->name);
	} else {
		obuf += snprintf(obuf, dry_run ? bufsz - (obuf-buf) : 0,
		    ">" NL);
		/* print all children */
		XMLSD_ELEM_FOREACH_CHILDREN(xc, xe) {
			obuf += xmlsd_generate_elem(xc, obuf,
			    dry_run ? bufsz - (obuf - buf) : 0, dry_run);
		}
		/* close our tag */
		obuf += snprintf(obuf, dry_run ? bufsz - (obuf-buf) : 0,
		    "%*s</%s>" NL, xe->depth * 2, "", xe->name);
	}
	return (obuf-buf);
}

char *
xmlsd_generate(struct xmlsd_document *xd, void *(*alloc_fn)(size_t),
    size_t *xmlszp, int flags)
{
	char			*obuf, *buf;
	size_t			bufsz;

	obuf = NULL;
	bufsz = 0;
	if (xmlszp != NULL)
		*xmlszp = -1;

	if (xd == NULL || alloc_fn == NULL)
		return NULL;
for_real:
	buf = obuf;

	if (flags & XMLSD_GEN_ADD_HEADER) {
		obuf += snprintf(obuf, buf ? bufsz - (obuf-buf) : 0,
		    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" NL NL);
	}
	if (xd->root != NULL)
		obuf += xmlsd_generate_elem(xd->root, obuf, buf ?
		    bufsz - (obuf - buf) : 0, buf != NULL);
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


#if 0
tag = xmld_add_tag(top, "file");

top = xmlsd_create(&xl, "ct_md_open_read");

top = xmlsd_create("ct_md_open_read");
xmlsd_elem_set_attr(top, "version", STR(CT_MD_OPEN_READ_VERSION));
tag = xmld_add_tag(top, "file");
xmlsd_elem_set_attr(tag, "name", string);
xml = xmlsd_generate(top);
#endif
