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

#include "xmlsd.h"

#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <poll.h>
#include <expat.h>

static const char *vertag = "version: " XMLSD_VERSION;

#define XMLSD_PAGE_SIZE		(1024)
#define XML_MAX_PAGE_SIZE	(4 * XMLSD_PAGE_SIZE)

struct xmlsd_context {
	XML_Parser			xml_parser;
	XML_Char			*value;
	int				value_at;
	int				tot_size;
	int				depth;

	struct xmlsd_element_list	*xml_el;
};

static int			xmlsd_error = XMLSD_ERR_UNKNOWN;
#define XMLSD_ABORT(_xml, _rv)	do {					\
					xmlsd_error = _rv;		\
					XML_StopParser(_xml, XML_FALSE);\
					return;				\
				} while (0)

void
xmlsd_version(int *major, int *minor, int *patch)
{
	*major = XMLSD_VERSION_MAJOR;
	*minor = XMLSD_VERSION_MINOR;
	*patch = XMLSD_VERSION_PATCH;
	/* Portable way to avoid unused variable compile warnings */
	(void) (vertag);
}

void
xmlsd_chardata(void *data, const XML_Char *s, int len)
{
	int			newlen;
	XML_Char		*newvalue;
	XML_Parser		xml;
	struct	xmlsd_context	*ctx = data;

	if (ctx == NULL)
		errx(1, "xmlsd_chardata: no context");
	xml = ctx->xml_parser;

	/* make sure it isn't EOL */
	if (iscntrl(s[0]) && len == 1)
		return;

	if (ctx->value == NULL) {
		/* eat all blanks in front because expat isn't smart */
		while (isblank(s[0])) {
			s += 1;
			len -= 1;
		}
		if (len == 0)
			return;

		ctx->value = calloc(1, XMLSD_PAGE_SIZE);
		if (ctx->value == NULL)
			XMLSD_ABORT(xml, XMLSD_ERR_RESOURCE);
		ctx->tot_size += XMLSD_PAGE_SIZE;
	}

	/* check for overflow */
	if (ctx->value_at + len > ctx->tot_size) {
		for (newlen = ctx->tot_size; newlen < ctx->value_at + len;
		    newlen += XMLSD_PAGE_SIZE)
			;

		if (newlen > XML_MAX_PAGE_SIZE) {
			XMLSD_ABORT(xml, XMLSD_ERR_OVERFLOW);
			return;
		}

		newvalue = realloc(ctx->value, newlen);
		if (newvalue == NULL) {
			free(ctx->value);
			ctx->value = NULL;
			ctx->value_at = 0;
			ctx->tot_size = 0;
			XMLSD_ABORT(xml, XMLSD_ERR_RESOURCE);
		}
		ctx->value = newvalue;
		ctx->tot_size = newlen;
	}

	/* copy new content where we left of */
	bcopy(s, &ctx->value[ctx->value_at], len);
	ctx->value_at += len;
	ctx->value[ctx->value_at] = '\0';
}

void
xmlsd_start(void *data, const char *el, const char **attr)
{
	int			i;
	XML_Parser		xml;
	struct xmlsd_context	*ctx = data;
	struct xmlsd_element	*xe, *x;
	struct xmlsd_attribute	*xa;

	if (ctx == NULL)
		errx(1, "xmlsd_start: no context");
	xml = ctx->xml_parser;

	xe = calloc(1, sizeof *xe);
	if (xe == NULL)
		XMLSD_ABORT(xml, XMLSD_ERR_RESOURCE);

	TAILQ_INSERT_TAIL(ctx->xml_el, xe, entry);
	xe->name = strdup(el);
	if (xe->name == NULL)
		XMLSD_ABORT(xml, XMLSD_ERR_RESOURCE);
	TAILQ_INIT(&xe->attr_list);

	for (i = 0; attr[i]; i += 2) {
		/*fprintf(stderr, "%s -> %s = %s\n", el, attr[i], attr[i + 1]);*/
		xa = calloc(1, sizeof *xa);
		if (xa == NULL)
			XMLSD_ABORT(xml, XMLSD_ERR_RESOURCE);
		xa->name = strdup(attr[i]);
		if (xa->name == NULL)
			XMLSD_ABORT(xml, XMLSD_ERR_RESOURCE);
		xa->value = strdup(attr[i + 1]);
		if (xa->value == NULL)
			XMLSD_ABORT(xml, XMLSD_ERR_RESOURCE);
		TAILQ_INSERT_TAIL(&xe->attr_list, xa, entry);
	}

	ctx->depth++;
	xe->depth = ctx->depth;

	/* find parent */
	xe->parent = NULL;
	TAILQ_FOREACH_REVERSE(x, ctx->xml_el, xmlsd_element_list, entry) {
		if (x->depth < xe->depth) {
			xe->parent = x;
			break;
		}
	}
}

void
xmlsd_end(void *data, const char *el)
{
	XML_Parser		xml;
	struct xmlsd_context	*ctx = data;
	struct xmlsd_element	*xe;

	if (ctx == NULL)
		errx(1, "xmlsd_end: no context");
	xml = ctx->xml_parser;

	if (ctx->value) {
		if (ctx->value_at > 1) {
			/* eat all blanks in back because expat isn't smart */
			while (isblank(ctx->value[--ctx->value_at])) {
				ctx->value[ctx->value_at] = '\0';
				if (ctx->value_at == 0)
					break;
			}
		}
		/* save off value */
		xe = TAILQ_LAST(ctx->xml_el, xmlsd_element_list);
		if (xe == NULL)
			XMLSD_ABORT(xml, XMLSD_ERR_INTEGRITY);
		if (strcmp(xe->name, el))
			XMLSD_ABORT(xml, XMLSD_ERR_INTEGRITY);
		if (xe->value)
			XMLSD_ABORT(xml, XMLSD_ERR_INTEGRITY);
		xe->value = strdup(ctx->value);
		if (xe->value == NULL)
			XMLSD_ABORT(xml, XMLSD_ERR_RESOURCE);

		free(ctx->value);
		ctx->value = NULL;
		ctx->value_at = 0;
		ctx->tot_size = 0;
	}

	ctx->depth--;
}

int
xmlsd_parse_setup(struct xmlsd_context *ctx, struct xmlsd_element_list *xl)
{
	XML_Parser		xml;

	if (ctx == NULL || (xl && !TAILQ_EMPTY(xl)))
		return (XMLSD_ERR_INTEGRITY);

	bzero(ctx, sizeof *ctx);
	ctx->depth = -1;

	xml = XML_ParserCreate(NULL);
	if (xml == NULL)
		return (XMLSD_ERR_RESOURCE);

	ctx->xml_parser = xml;
	ctx->xml_el = xl;

	XML_SetUserData(xml, ctx);
	XML_SetElementHandler(xml, xmlsd_start, xmlsd_end);
	XML_SetCharacterDataHandler(xml, xmlsd_chardata);

	return (XMLSD_ERR_SUCCES);
}

void
xmlsd_parse_done(struct xmlsd_context *ctx)
{
	XML_ParserFree(ctx->xml_parser);
}

int
xmlsd_parse_fileds(int f, struct xmlsd_element_list *xl)
{
	XML_Parser		xml;
	struct xmlsd_context	ctx;
	int			irv, done, rv = XMLSD_ERR_UNKNOWN, status;
	size_t			r;
	char			b[XMLSD_PAGE_SIZE];
	struct pollfd		fds[1];

	errx(1, "xmlsd_parse_fileds: UNTESTED");

	if (f <= 0 || xl == NULL)
		return (XMLSD_ERR_INTEGRITY);

	if ((irv = xmlsd_parse_setup(&ctx, xl)) != XMLSD_ERR_SUCCES)
		return (irv);

	xml = ctx.xml_parser;
	for (done = 0; done == 0;) {
		fds[0].fd = f;
		fds[0].events = POLLIN;
		irv = poll(fds, 1, XMLSD_TIMEOUT);
		if (irv == -1) {
			if (errno == EINTR || errno == EAGAIN) {
				fprintf(stderr, "poll %d", errno);
				continue;
			}
			if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
				goto done;
		}
		if (irv == 0)
			goto done;

		r = read(f, b, sizeof b);
		if (r == -1) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			goto done;
		}
		if (r == 0)
			done = 1;

		if (XML_Parse(xml, b, r, done) != XML_STATUS_OK) {
			status = XML_GetErrorCode(xml);
			if (status == XML_ERROR_ABORTED)
				rv = xmlsd_error;
			else
				rv = XMLSD_ERR_PARSER;
			goto done;
		}
	}

	rv = XMLSD_ERR_SUCCES;
done:
	xmlsd_parse_done(&ctx);
	return (rv);
}

int
xmlsd_parse_file(FILE *f, struct xmlsd_element_list *xl)
{
	XML_Parser		xml;
	struct xmlsd_context	ctx;
	int			irv, done, rv = XMLSD_ERR_UNKNOWN, status;
	size_t			r;
	char			b[XMLSD_PAGE_SIZE];

	if (f == NULL || xl == NULL)
		return (XMLSD_ERR_INTEGRITY);

	if ((irv = xmlsd_parse_setup(&ctx, xl)) != XMLSD_ERR_SUCCES)
		return (irv);

	xml = ctx.xml_parser;
	for (done = 0; done == 0;) {
		r = fread(b, 1, sizeof b, f);
		if (ferror(f)) {
			rv = XMLSD_ERR_EXTERNAL;
			goto done;
		}
		done = feof(f);
		if (XML_Parse(xml, b, r, done) != XML_STATUS_OK) {
			status = XML_GetErrorCode(xml);
			if (status == XML_ERROR_ABORTED)
				rv = xmlsd_error;
			else
				rv = XMLSD_ERR_PARSER;
			goto done;
		}
	}

	rv = XMLSD_ERR_SUCCES;
done:
	xmlsd_parse_done(&ctx);
	return (rv);
}

int
xmlsd_parse_mem(char *b, size_t sz, struct xmlsd_element_list *xl)
{
	int			irv, status, rv = XMLSD_ERR_UNKNOWN;
	struct xmlsd_context	ctx;
	XML_Parser		xml;

	if (b == NULL || sz <= 0 || xl == NULL)
		return (XMLSD_ERR_INTEGRITY);

	if ((irv = xmlsd_parse_setup(&ctx, xl)) != XMLSD_ERR_SUCCES)
		return (irv);

	xml = ctx.xml_parser;
	if (XML_Parse(xml, b, sz, 1) != XML_STATUS_OK) {
		status = XML_GetErrorCode(xml);
		if (status == XML_ERROR_ABORTED)
			rv = xmlsd_error;
		else
			rv = XMLSD_ERR_PARSER;
		goto done;
	}

	rv = XMLSD_ERR_SUCCES;
done:
	xmlsd_parse_done(&ctx);
	return (rv);
}

int
xmlsd_unwind(struct xmlsd_element_list *xl)
{
	struct xmlsd_element *xe;

	if (xl == NULL)
		return (XMLSD_ERR_INTEGRITY);

	while ((xe = TAILQ_FIRST(xl))) {
		xmlsd_remove_element(xl, xe);
	}

	return (XMLSD_ERR_SUCCES);
}

int
xmlsd_check_path(struct xmlsd_element *xe, char *path)
{
	int			rv = 1;
	struct xmlsd_element	*current;
	char			mypath[1024] = { '\0' };

	if (xe == NULL || path == NULL)
		goto done;

	/* validate root element */
	if (strlen(path) == 0 && xe->parent == NULL)
		return (0);

	current = xe;
	while (current) {
		if (xe != current)
			strlcat(mypath, ".", sizeof mypath);
		if (strlcat(mypath, current->name, sizeof mypath) >=
		    sizeof mypath)
			goto done;
		current = current->parent;
	}

	if (strcmp(mypath, path))
		goto done;

	rv = 0;
done:
	return (rv);
}

int
xmlsd_check_attributes(struct xmlsd_element *xe, struct xmlsd_v_attr *attrs)
{
	struct xmlsd_attribute	*xa;
	int			i, found, rv = 1;

	if (!attrs)
		goto done;

	TAILQ_FOREACH(xa, &xe->attr_list, entry) {
		found = 0;
		for (i = 0; attrs[i].name != NULL; i++)
			if (!strcmp(attrs[i].name, xa->name)) {
				found = 1;
				break;
			}

		if (found == 0)
			goto done;
	}

	rv = 0;
done:
	return (rv);
}

int
xmlsd_validate(struct xmlsd_element_list *xl, struct xmlsd_v_elements *els)
{
	struct xmlsd_element	*xe;
	struct xmlsd_v_elem	*xc = NULL, *cmd;
	int			i, rv = 1;

	if (TAILQ_EMPTY(xl))
		goto done;

	/* find command */
	xe = TAILQ_FIRST(xl);
	if (xe == NULL)
		goto done;

	/* must not have a parent */
	if (xe->parent)
		goto done;

	for (i = 0; els[i].name != NULL; i++)
		if (!strcmp(els[i].name, xe->name))
			xc = els[i].cmd;
	if (xc == NULL)
		goto done;

	/* make sure we are the root element */
	if (xmlsd_check_path(xe, xc->path))
		goto done;

	/* check root attributes */
	if (!TAILQ_EMPTY(&xe->attr_list))
		if (xmlsd_check_attributes(xe, xc->attr))
			goto done;

	TAILQ_FOREACH(xe, xl, entry) {
		/* skip first */
		if (xe == TAILQ_FIRST(xl))
			continue;

		/* find element */
		for (cmd = NULL, i = 0; xc[i].element != NULL; i++)
			if (!strcmp(xc[i].element, xe->name) &&
			    !xmlsd_check_path(xe, xc[i].path))
				cmd = &xc[i];
		if (cmd == NULL)
			goto done;

		/* check attributes */
		if (!TAILQ_EMPTY(&xe->attr_list))
			if (xmlsd_check_attributes(xe, cmd->attr))
				goto done;
	}

	rv = 0;
done:
	return (rv);
}

char *
xmlsd_get_value(struct xmlsd_element_list *xl, char *findme,
    struct xmlsd_element **xe_ret)
{
	struct xmlsd_element	*xe;

	if (xl == NULL || findme == NULL)
		return (NULL);

	TAILQ_FOREACH(xe, xl, entry) {
		if (!strcmp(xe->name, findme)) {
			if (xe_ret)
				*xe_ret = xe;
			return (xe->value);
		}
	}

	return (NULL);
}

char *
xmlsd_get_attr(struct xmlsd_element *xe, char *findme)
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

int
xmlsd_check_boolean(char *s, int *v)
{
	int			rv = 1, r = -1;

	if (s == NULL)
		goto done;

	if (!strcmp(s, "true") || !strcmp(s, "1"))
		r = 1;
	else if (!strcmp(s, "false") || !strcmp(s, "0"))
		r = 0;
	else
		goto done;

	if (r == -1)
		goto done;

	if (v)
		*v = r;

	rv = 0;
done:
	return (rv);
}
