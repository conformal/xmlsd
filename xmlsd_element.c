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
#include <errno.h>
#include <limits.h>

#include "xmlsd.h"
#include "xmlsd_internal.h"

#include <string.h>

const char *
xmlsd_elem_get_name(struct xmlsd_element *xe)
{
	return (xe->name);
}

int
xmlsd_elem_get_depth(struct xmlsd_element *xe)
{
	return (xe->depth);
}

struct xmlsd_element *
xmlsd_elem_get_parent(struct xmlsd_element *xe)
{
	return (xe->parent);
}

struct xmlsd_attribute	*
xmlsd_elem_find_attr(struct xmlsd_element *xe, const char *findme)
{
	struct xmlsd_attribute	*xa;

	TAILQ_FOREACH(xa, &xe->attr_list, entry) {
		if (!strcmp(xe->name, findme)) 
			break;
	}

	return (xa);
}

struct xmlsd_attribute	*
xmlsd_elem_get_first_attr(struct xmlsd_element *xe)
{
	return (TAILQ_FIRST(&xe->attr_list));
}

struct xmlsd_attribute	*
xmlsd_elem_get_next_attr(struct xmlsd_element *xe, struct xmlsd_attribute *xa)
{
	return (TAILQ_NEXT(xa, entry));
}

struct xmlsd_attribute	*
xmlsd_elem_get_last_attr(struct xmlsd_element *xe)
{
	return (TAILQ_LAST(&xe->attr_list, xmlsd_attribute_list));
}

struct xmlsd_attribute	*
xmlsd_elem_get_previous_attr(struct xmlsd_element *xe,
    struct xmlsd_attribute *xa)
{
	return (TAILQ_PREV(xa, xmlsd_attribute_list, entry));
}

struct xmlsd_element	*
xmlsd_elem_find_child(struct xmlsd_element *xe, const char *findme)
{
	struct xmlsd_element	*xc;

	if (xe == NULL || findme == NULL)
		return (NULL);
	TAILQ_FOREACH(xc, &xe->children, entry) {
		if (!strcmp(xc->name, findme))
			break;
	}

	return (xc);
}

struct xmlsd_element	*
xmlsd_elem_get_first_child(struct xmlsd_element *xe)
{
	return (TAILQ_FIRST(&xe->children));
}

struct xmlsd_element	*
xmlsd_elem_get_next_child(struct xmlsd_element *xe, struct xmlsd_element *xc)
{
	return (TAILQ_NEXT(xc, entry));
}

struct xmlsd_element	*
xmlsd_elem_get_last_child(struct xmlsd_element *xe)
{
	return (TAILQ_LAST(&xe->children, xmlsd_element_list));
}

struct xmlsd_element	*
xmlsd_elem_get_previous_child(struct xmlsd_element *xe,
    struct xmlsd_element *xc)
{
	return (TAILQ_PREV(xc, xmlsd_element_list, entry));
}

const char *
xmlsd_elem_get_attr(struct xmlsd_element *xe, const char *findme)
{
	struct xmlsd_attribute	*xa;

	if (xe == NULL || findme == NULL)
		return (NULL);

	TAILQ_FOREACH(xa, &xe->attr_list, entry) {
		if (!strcmp(xa->name, findme))
			return (xa->value);
	}
	return (NULL);
}

long long
xmlsd_elem_get_attr_strtonum(struct xmlsd_element *xe, const char *attr,
    long long minval, long long maxval, const char **errstr)
{
	const char	*str;

	if ((str = xmlsd_elem_get_attr(xe, attr)) == NULL) {
	    *errstr = "attr not found";
	    return (0);
	}

	return (strtonum(str, minval, maxval, errstr));
}

unsigned long long
xmlsd_elem_get_attr_hexnum(struct xmlsd_element *xe, const char *attr,
    unsigned long long minval, unsigned long long maxval, const char **errstr)
{
	const char		*str;
	char			*end;
	unsigned long long	 val = 0;

	*errstr = NULL;
	if ((str = xmlsd_elem_get_attr(xe, attr)) == NULL) {
	    *errstr = "attr not found";
	    return (0);
	}
	if (minval > maxval) {
		*errstr = "invalid";
	} else {
		val = strtoull(str, &end, 16);
		if (str == end || *end != '\0')
			*errstr = "invalid";
		else if (val < minval)
			*errstr = "toosmall";
		else if ((val == ULLONG_MAX && errno == ERANGE) ||
		    val < maxval)
			*errstr = "too large";
	}
	if (*errstr != NULL)
		val = 0;
	return (val);
}

/*
 * Get attr ``name'' from elem. asa boolean value.
 *
 * If no attr is present then return ``def'' instead.
 */
int
xmlsd_elem_get_attr_boolean(struct xmlsd_element *elem, const char *name,
    int *ret_b, int def)
{
	const char	*attr;
	int		 rv = 0;

	if (elem == NULL)
		return (XMLSD_ERR_INTEGRITY);
	if (name == NULL)
		return (XMLSD_ERR_INTEGRITY);
	if (ret_b == NULL)
		return (XMLSD_ERR_INTEGRITY);

	if ((attr = xmlsd_elem_get_attr(elem, name)) == NULL) {
		*ret_b = def;
		return (0);
	}

	if (!strcmp(attr, "true") || !strcmp(attr, "1"))
		*ret_b = 1;
	else if (!strcmp(attr, "false") || !strcmp(attr, "0"))
		*ret_b = 0;
	else
		rv = XMLSD_ERR_INTEGRITY;

	return (rv);
}

int
xmlsd_elem_set_attr_int32(struct xmlsd_element *xe, const char *name,
    int32_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL || name == NULL || (strlen(name) == 0))
		return 1;

	if (asprintf(&buf, "%d", ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_attr(xe, name, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_attr_uint32(struct xmlsd_element *xe, const char *name,
    uint32_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL || name == NULL || (strlen(name) == 0))
		return 1;

	if (asprintf(&buf, "%u", ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_attr(xe, name, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_attr_int64(struct xmlsd_element *xe, const char *name,
    int64_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL || name == NULL || (strlen(name) == 0))
		return 1;

	if (asprintf(&buf, "%" PRId64, ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_attr(xe, name, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_attr_uint64(struct xmlsd_element *xe, const char *name,
    uint64_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL || name == NULL || (strlen(name) == 0))
		return 1;

	if (asprintf(&buf, "%" PRIu64, ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_attr(xe, name, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_attr_x32(struct xmlsd_element *xe, const char *name,
    uint32_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL || name == NULL || (strlen(name) == 0))
		return 1;

	if (asprintf(&buf, "0x%x", ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_attr(xe, name, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_attr_x64(struct xmlsd_element *xe, const char *name,
    uint64_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL || name == NULL || (strlen(name) == 0))
		return 1;

	if (asprintf(&buf, "0x%" PRIx64, ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_attr(xe, name, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_attr(struct xmlsd_element *xe, const char *name,
    const char *value)
{
	struct xmlsd_attribute *xa = NULL;

	if (xe == NULL || name == NULL || (strlen(name) == 0) || value == NULL)
		goto fail;

        xa = calloc(1, sizeof *xa);

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
		free(xa);
	}
	return 1;
}

const char *
xmlsd_elem_get_value(struct xmlsd_element *xe)
{
	if (xe == NULL)
		return (NULL);

	return (xe->value);
}

long long
xmlsd_elem_get_value_strtonum(struct xmlsd_element *xe, long long minval,
    long long maxval, const char **errstr)
{
	const char	*str;

	if ((str = xmlsd_elem_get_value(xe)) == NULL) {
	    *errstr = "no value found";
	    return (0);
	}

	return (strtonum(str, minval, maxval, errstr));
}

unsigned long long
xmlsd_elem_get_value_hexnum(struct xmlsd_element *xe,
    unsigned long long minval, unsigned long long maxval, const char **errstr)
{
	const char		*str;
	char			*end;
	unsigned long long	 val = 0;

	*errstr = NULL;
	if ((str = xmlsd_elem_get_value(xe)) == NULL) {
	    *errstr = "no value found";
	    return (0);
	}
	if (minval > maxval) {
		*errstr = "invalid";
	} else {
		val = strtoull(str, &end, 16);
		if (str == end || *end != '\0')
			*errstr = "invalid";
		else if (val < minval)
			*errstr = "toosmall";
		else if ((val == ULLONG_MAX && errno == ERANGE) ||
		    val < maxval)
			*errstr = "too large";
	}
	if (*errstr != NULL)
		val = 0;
	return (val);
}

/*
 * Get attr ``name'' from elem. asa boolean value.
 *
 * If no attr is present then return ``def'' instead.
 */
int
xmlsd_elem_get_value_boolean(struct xmlsd_element *elem, int *ret_b, int def)
{
	const char	*value;
	int		 rv = 0;

	if (elem == NULL)
		return (XMLSD_ERR_INTEGRITY);
	if (ret_b == NULL)
		return (XMLSD_ERR_INTEGRITY);

	if ((value = xmlsd_elem_get_value(elem)) == NULL) {
		*ret_b = def;
		return (0);
	}

	if (!strcmp(value, "true") || !strcmp(value, "1"))
		*ret_b = 1;
	else if (!strcmp(value, "false") || !strcmp(value, "0"))
		*ret_b = 0;
	else
		rv = XMLSD_ERR_INTEGRITY;

	return (rv);
}

int
xmlsd_elem_set_value_int32(struct xmlsd_element *xe, int32_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL)
		return 1;

	if (asprintf(&buf, "%d", ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_value(xe, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_value_uint32(struct xmlsd_element *xe, uint32_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL)
		return 1;

	if (asprintf(&buf, "%u", ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_value(xe, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_value_int64(struct xmlsd_element *xe, int64_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL)
		return 1;

	if (asprintf(&buf, "%" PRId64, ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_value(xe, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_value_uint64(struct xmlsd_element *xe, uint64_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL)
		return 1;

	if (asprintf(&buf, "%" PRIu64, ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_value(xe, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_value_x32(struct xmlsd_element *xe, uint32_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL)
		return 1;

	if (asprintf(&buf, "0x%x", ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_value(xe, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_value_x64(struct xmlsd_element *xe, uint64_t ival)
{
	char *buf;
	int rv;

	if (xe == NULL)
		return 1;

	if (asprintf(&buf, "0x%" PRIx64, ival) == -1)
		return 1;
	if (buf == NULL)
		return 1;

	rv = xmlsd_elem_set_value(xe, buf);

	free(buf);

	return rv;
}

int
xmlsd_elem_set_value(struct xmlsd_element *xe, const char *value)
{
	if (xe == NULL || value == NULL)
		return 1;

	if (xe->value)
		free(xe->value);

	xe->value = strdup(value);
	if (xe->value == NULL)
		return 1;

	return 0;
}

void
xmlsd_elem_free(struct xmlsd_element *xe)
{
	struct xmlsd_attribute	*xa;

	if (xe == NULL)
		return;

	/* free attributes */
	while ((xa = TAILQ_FIRST(&xe->attr_list))) {
		TAILQ_REMOVE(&xe->attr_list, xa, entry);
		if (xa->name)
			free(xa->name);
		if (xa->value)
			free(xa->value);
		free(xa);
	}

	/* free element */
	if (xe->name)
		free(xe->name);
	if (xe->value)
		free(xe->value);

	free(xe);
}

