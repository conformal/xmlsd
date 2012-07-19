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
#include "xmlsd_internal.h"

#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <poll.h>
#include <expat.h>

#ifdef BUILDSTR
static const char *vertag = "version: " XMLSD_VERSION " " BUILDSTR;
#else
static const char *vertag = "version: " XMLSD_VERSION;
#endif


#define XMLSD_PAGE_SIZE		(1024)
#define XML_MAX_PAGE_SIZE	(4 * XMLSD_PAGE_SIZE)

struct xmlsd_context {
	XML_Parser			xml_parser;
	XML_Char			*value;
	int				value_at;
	int				tot_size;
	int				depth;
	int				saved_rv;

	struct xmlsd_document		*xml_el;
	struct xmlsd_element		*xml_last;
};

#define XMLSD_ABORT(_ctx, _rv)	do {			\
	(_ctx)->saved_rv = _rv;				\
	XML_StopParser((_ctx)->xml_parser, XML_FALSE);	\
	return;						\
} while (0)

static int	xmlsd_calc_path(struct xmlsd_element *, char *, size_t);
static void	xmlsd_chardata(void *, const XML_Char *, int);
static void	xmlsd_end(void *, const char *);
static int	xmlsd_occurrences(struct xmlsd_document *,
		    struct xmlsd_element *, const char *);
static void	xmlsd_parse_done(struct xmlsd_context *);
static int	xmlsd_parse_setup(struct xmlsd_context *,
		    struct xmlsd_document *);
static void	xmlsd_start(void *, const char *, const char **);

const char *
xmlsd_verstring()
{
	return (vertag);
}

void
xmlsd_version(int *major, int *minor, int *patch)
{
	*major = XMLSD_VERSION_MAJOR;
	*minor = XMLSD_VERSION_MINOR;
	*patch = XMLSD_VERSION_PATCH;
}

static void
xmlsd_chardata(void *data, const XML_Char *s, int len)
{
	int			newlen;
	XML_Char		*newvalue;
	struct	xmlsd_context	*ctx = data;

	if (ctx == NULL)
		errx(1, "xmlsd_chardata: no context");

	/* make sure it isn't EOL */
	if (iscntrl(s[0]) && len == 1)
		return;

	if (ctx->value == NULL) {
		/* eat all blanks in front because expat isn't smart */
		while (len > 0 && isblank(s[0])) {
			s += 1;
			len -= 1;
		}
		if (len == 0)
			return;

		ctx->value = calloc(1, XMLSD_PAGE_SIZE);
		if (ctx->value == NULL)
			XMLSD_ABORT(ctx, XMLSD_ERR_RESOURCE);
		ctx->tot_size += XMLSD_PAGE_SIZE;
	}

	/* check for overflow (DO NOT FORGET NUL!) */
	if (ctx->value_at + len + 1 > ctx->tot_size) {
		for (newlen = ctx->tot_size; newlen < ctx->value_at + len + 1;
		    newlen += XMLSD_PAGE_SIZE)
			;

		if (newlen > XML_MAX_PAGE_SIZE) {
			XMLSD_ABORT(ctx, XMLSD_ERR_OVERFLOW);
			return;
		}

		newvalue = realloc(ctx->value, newlen);
		if (newvalue == NULL) {
			free(ctx->value);
			ctx->value = NULL;
			ctx->value_at = 0;
			ctx->tot_size = 0;
			XMLSD_ABORT(ctx, XMLSD_ERR_RESOURCE);
		}
		ctx->value = newvalue;
		ctx->tot_size = newlen;
	}

	/* copy new content where we left of */
	bcopy(s, &ctx->value[ctx->value_at], len);
	ctx->value_at += len;
	ctx->value[ctx->value_at] = '\0';
}

static void
xmlsd_start(void *data, const char *el, const char **attr)
{
	int			i;
	struct xmlsd_context	*ctx = data;
	struct xmlsd_element	*xe;
	struct xmlsd_attribute	*xa;

	if (ctx == NULL)
		errx(1, "xmlsd_start: no context");

	xe = calloc(1, sizeof *xe);
	if (xe == NULL)
		XMLSD_ABORT(ctx, XMLSD_ERR_RESOURCE);

	if (ctx->xml_last != NULL) {
		TAILQ_INSERT_TAIL(&ctx->xml_last->children, xe, entry);
	} else { /* top level */
		TAILQ_INSERT_TAIL(&ctx->xml_el->children, xe, entry);
	}
	xe->name = strdup(el);
	if (xe->name == NULL)
		XMLSD_ABORT(ctx, XMLSD_ERR_RESOURCE);
	TAILQ_INIT(&xe->children);
	xe->parent = ctx->xml_last;
	ctx->xml_last = xe;
	TAILQ_INIT(&xe->attr_list);

	for (i = 0; attr[i]; i += 2) {
		/*fprintf(stderr, "%s -> %s = %s\n", el, attr[i], attr[i + 1]);*/
		xa = calloc(1, sizeof *xa);
		if (xa == NULL)
			XMLSD_ABORT(ctx, XMLSD_ERR_RESOURCE);
		xa->name = strdup(attr[i]);
		if (xa->name == NULL)
			XMLSD_ABORT(ctx, XMLSD_ERR_RESOURCE);
		xa->value = strdup(attr[i + 1]);
		if (xa->value == NULL)
			XMLSD_ABORT(ctx, XMLSD_ERR_RESOURCE);
		TAILQ_INSERT_TAIL(&xe->attr_list, xa, entry);
	}

	ctx->depth++;
	xe->depth = ctx->depth;
}

static void
xmlsd_end(void *data, const char *el)
{
	struct xmlsd_context	*ctx = data;
	struct xmlsd_element	*xe;

	if (ctx == NULL)
		errx(1, "xmlsd_end: no context");

	xe = ctx->xml_last;
	if (xe == NULL)
		XMLSD_ABORT(ctx, XMLSD_ERR_INTEGRITY);
	if (strcmp(xe->name, el))
		XMLSD_ABORT(ctx, XMLSD_ERR_INTEGRITY);
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
		if (xe->value)
			XMLSD_ABORT(ctx, XMLSD_ERR_INTEGRITY);
		xe->value = strdup(ctx->value);
		if (xe->value == NULL)
			XMLSD_ABORT(ctx, XMLSD_ERR_RESOURCE);

		free(ctx->value);
		ctx->value = NULL;
		ctx->value_at = 0;
		ctx->tot_size = 0;
	}

	/* go up a level */
	ctx->depth--;
	ctx->xml_last = xe->parent;
}

static int
xmlsd_parse_setup(struct xmlsd_context *ctx, struct xmlsd_document *xd)
{
	XML_Parser		xml;

	if (ctx == NULL || (xd && !xmlsd_doc_is_empty(xd)))
		return (XMLSD_ERR_INTEGRITY);

	bzero(ctx, sizeof *ctx);
	ctx->depth = -1;
	ctx->saved_rv = XMLSD_ERR_UNKNOWN;

	xml = XML_ParserCreate(NULL);
	if (xml == NULL)
		return (XMLSD_ERR_RESOURCE);

	ctx->xml_parser = xml;
	ctx->xml_el = xd;
	ctx->xml_last = NULL;

	XML_SetUserData(xml, ctx);
	XML_SetElementHandler(xml, xmlsd_start, xmlsd_end);
	XML_SetCharacterDataHandler(xml, xmlsd_chardata);

	return (XMLSD_ERR_SUCCES);
}

static void
xmlsd_parse_done(struct xmlsd_context *ctx)
{
	XML_ParserFree(ctx->xml_parser);
}

int
xmlsd_parse_fileds(int f, struct xmlsd_document *xd)
{
	XML_Parser		xml;
	struct xmlsd_context	ctx;
	int			irv, done, rv = XMLSD_ERR_UNKNOWN, status;
	ssize_t			r;
	char			b[XMLSD_PAGE_SIZE];
	struct pollfd		fds[1];

	errx(1, "xmlsd_parse_fileds: UNTESTED");

	if (f <= 0 || xd == NULL)
		return (XMLSD_ERR_INTEGRITY);

	if ((irv = xmlsd_parse_setup(&ctx, xd)) != XMLSD_ERR_SUCCES)
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
				rv = ctx.saved_rv;
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
xmlsd_parse_file(FILE *f, struct xmlsd_document *xd)
{
	XML_Parser		xml;
	struct xmlsd_context	ctx;
	int			irv, done, rv = XMLSD_ERR_UNKNOWN, status;
	size_t			r;
	char			b[XMLSD_PAGE_SIZE];

	if (f == NULL || xd == NULL)
		return (XMLSD_ERR_INTEGRITY);

	if ((irv = xmlsd_parse_setup(&ctx, xd)) != XMLSD_ERR_SUCCES)
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
				rv = ctx.saved_rv;
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
xmlsd_parse_mem(const char *b, size_t sz, struct xmlsd_document *xd)
{
	int			irv, status, rv = XMLSD_ERR_UNKNOWN;
	struct xmlsd_context	ctx;
	XML_Parser		xml;

	if (b == NULL || sz <= 0 || xd == NULL)
		return (XMLSD_ERR_INTEGRITY);

	if ((irv = xmlsd_parse_setup(&ctx, xd)) != XMLSD_ERR_SUCCES)
		return (irv);

	xml = ctx.xml_parser;
	if (XML_Parse(xml, b, sz, 1) != XML_STATUS_OK) {
		status = XML_GetErrorCode(xml);
		if (status == XML_ERROR_ABORTED)
			rv = ctx.saved_rv;
		else
			rv = XMLSD_ERR_PARSER;
		goto done;
	}

	rv = XMLSD_ERR_SUCCES;
done:
	xmlsd_parse_done(&ctx);
	return (rv);
}

static int
xmlsd_calc_path(struct xmlsd_element *xe, char *mypath, size_t mypathlen)
{
	struct xmlsd_element	*current;

	mypath[0] = '\0';
	for (current = xe; current != NULL; current = current->parent) {
		if (xe != current)
			strlcat(mypath, ".", mypathlen);
		if (strlcat(mypath, current->name, mypathlen) >= mypathlen)
			return 1;	/* Insufficient space. */
	}

	return 0;
}

int
xmlsd_check_path(struct xmlsd_element *xe, char *path)
{
	int			rv = 1;
	char			mypath[1024];

	if (xe == NULL || path == NULL)
		goto done;

	/* validate root element */
	if (strlen(path) == 0 && xe->parent == NULL)
		return (0);

	if (xmlsd_calc_path(xe, mypath, sizeof mypath))
		goto done;
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

	if (!attrs) {
		if (TAILQ_EMPTY(&xe->attr_list)) {
			rv = 0;
		}
		goto done;
	}

	TAILQ_FOREACH(xa, &xe->attr_list, entry) {
		found = 0;
		for (i = 0; attrs[i].name != NULL; i++) {
			if (!strcmp(attrs[i].name, xa->name)) {
				found = 1;
				break;
			}
		}

		if (found == 0)
			goto done;
	}

	/* Required attribute verification. */
	for (i = 0; attrs[i].name != NULL; i++) {
		if (!(attrs[i].flags & XMLSD_V_ATTR_F_REQUIRED))
			continue;

		found = 0;
		TAILQ_FOREACH(xa, &xe->attr_list, entry) {
			if (!strcmp(attrs[i].name, xa->name)) {
				found = 1;
				break;
			}
		}

		if (found == 0)
			goto done;
	}

	rv = 0;
done:
	return (rv);
}

static int
xmlsd_occurrences(struct xmlsd_document *xl, struct xmlsd_element *parent,
    const char *name)
{
	struct xmlsd_element	*xi;
	int			 occur;

	/* Root node can only occur once by definition. */
	if (parent == NULL) {
		xi = xmlsd_doc_get_first_elem(xl);
		if (xi == NULL || !strcmp(xi->name, name))
			return 0;
		return 1;
	}

	occur = 0;
	XMLSD_ELEM_FOREACH_CHILDREN(xi, parent) {
		if (!strcmp(xi->name, name))
			occur++;
	}

	return occur;
}

/*
 * Validate an element and its children.
 *
 * `cmd' is the actual validation element that applies to this element.
 * `xc' is the list of elements for the whole document.
 */
static int
xmlsd_validate_element(struct xmlsd_document *xd, struct xmlsd_element *xe,
    struct xmlsd_v_elem *cmd, struct xmlsd_v_elem *xc)
{
	struct xmlsd_element	*xi;
	char			*dot;
	char			 xe_path[1024];
	int			 i, rv = 1, occur;

	/* check attributes */
	if (xmlsd_check_attributes(xe, cmd->attr)) {
		goto done;
	}

	/*
	 * Element occurrence validation.
	 */

	if (xmlsd_calc_path(xe, xe_path, sizeof xe_path)) {
		goto done;
	}
	for (i = 0; xc[i].element != NULL; i++) {
		if (xc[i].path == NULL)
			continue;	/* xc[i] is root. */
		if (xc[i].min_occurs == 0 && xc[i].max_occurs == 0)
			continue;	/* xc[i] occur irrelevant. */

		dot = strchr(xc[i].path, '.');
		if (dot == NULL)
			continue;	/* xc[i] is no child. */
		if (strcmp(xe_path, dot + 1))
			continue;	/* xc[i] is stranger child. */

		/*
		 * xc[i] is a child of xe and has occurrence
		 * constraints.
		 */
		occur = xmlsd_occurrences(xd, xe, xc[i].element);
		if (occur < xc[i].min_occurs)
			goto done;
		if (xc[i].max_occurs != 0 && occur > xc[i].max_occurs)
			goto done;
	}
	XMLSD_ELEM_FOREACH_CHILDREN(xi, xe) {
		for (cmd = NULL, i = 0; xc[i].element != NULL; i++)
			if (!strcmp(xc[i].element, xi->name) &&
			    !xmlsd_check_path(xi, xc[i].path))
				cmd = &xc[i];
		if (cmd == NULL) {
			goto done;
		}
		rv = xmlsd_validate_element(xd, xi, cmd, xc);
		if (rv != 0)
			goto done;
	}
	rv = 0;
done:
	return (rv);
}
/*
 * Validate XML
 * 
 * The structures used for validation assume a structure with one  top-level
 * command containing all other tags. Furthermore recursive structures
 * can not be defined and validated at this time.
 */
int
xmlsd_validate(struct xmlsd_document *xd, struct xmlsd_v_elements *els)
{
	struct xmlsd_element	*xe;
	struct xmlsd_v_elem	*xc = NULL, *cmd;
	int			 i, rv = 1;


	/* find command */
	if ((xe = xmlsd_doc_get_first_elem(xd)) == NULL)
		goto done;

	/* must not have a parent */
	if (xe->parent)
		goto done;

	for (i = 0; els[i].name != NULL; i++)
		if (!strcmp(els[i].name, xe->name))
			xc = els[i].cmd;
	if (xc == NULL)
		goto done;

	i = 0;
	XMLSD_DOC_FOREACH_ELEM(xe, xd) {
		/* should only be one of these... */
		if (++i > 1)
			goto done;
		cmd = xc;
		rv = xmlsd_validate_element(xd, xe, cmd, xc);
		if (rv)
			goto done;
	}

	rv = 0;
done:
	return (rv);
}

