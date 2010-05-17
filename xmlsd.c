/* $xmlsd$ */
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

static const char	*cvstag = "$xmlsd$";

#include "xmlsd.h"

#define XML_PAGE_SIZE		(1024)
#define XML_MAX_PAGE_SIZE	(4 * XML_PAGE_SIZE)

struct xmlsd_context {
	XML_Parser			xml_parser;
	XML_Char			*value;
	int				value_at;
	int				tot_size;
	int				depth;

	struct xmlsd_element_list	*xml_el;
};

void
xmlsd_chardata(void *data, const XML_Char *s, int len)
{
	int			newlen;
	XML_Char		*newvalue;
	XML_Parser		xml;
	struct	xmlsd_context	*ctx = data;

	if (ctx == NULL)
		errx(1, "xmlsd_chardata: no data");
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

		ctx->value = malloc(XML_PAGE_SIZE);
		if (ctx->value == NULL)
			err(1, "malloc");
		ctx->tot_size += XML_PAGE_SIZE;
	}

	/* check for overflow */
	if (ctx->value_at + len > ctx->tot_size) {
		for (newlen = ctx->tot_size; newlen < ctx->value_at + len;
		    newlen += XML_PAGE_SIZE)
			;

		if (newlen > XML_MAX_PAGE_SIZE)
			errx(1, "page too big");

		newvalue = realloc(ctx->value, newlen);
		if (newvalue == NULL) {
			free(ctx->value);
			ctx->value = NULL;
			ctx->value_at = 0;
			ctx->tot_size = 0;
			errx(1, "realloc");
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
		errx(1, "xmlsd_start: no data");
	xml = ctx->xml_parser;

	xe = malloc(sizeof *xe);
	if (xe == NULL)
		err(1, "xmlsd_start: malloc xe");
	TAILQ_INSERT_TAIL(ctx->xml_el, xe, entry);
	xe->name = strdup(el);
	if (xe->name == NULL)
		err(1, "xmlsd_start: strdup element name");
	TAILQ_INIT(&xe->attr_list);

	for (i = 0; attr[i]; i += 2) {
		//fprintf(stderr, "%s -> %s = %s\n", el, attr[i], attr[i + 1]);
		xa = malloc(sizeof *xa);
		if (xa == NULL)
			err(1, "xmlsd_start: malloc xa");
		xa->name = strdup(attr[i]);
		if (xa->name == NULL)
			err(1, "xmlsd_start: strdup attribute name");
		xa->value = strdup(attr[i + 1]);
		if (xa->value == NULL)
			err(1, "xmlsd_start: strdup attribute value");
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
		errx(1, "xmlsd_end: no data");
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
			errx(1, "xmlsd_end: TAILQ_LAST");
		if (strcmp(xe->name, el))
			errx(1, "xmlsd_end: invalid element");
		if (xe->value)
			errx(1, "xmlsd_end: %s already has a value", ctx->value);
		xe->value = strdup(ctx->value);
		if (xe->value == NULL)
			err(1, "xmlsd_end: strdup value");

		free(ctx->value);
		ctx->value = NULL;
		ctx->value_at = 0;
		ctx->tot_size = 0;
	}

	ctx->depth--;
}

int
xmlsd_parse(FILE *f, struct xmlsd_element_list *xl)
{
	XML_Parser		xml;
	struct xmlsd_context	ctx;
	int			done, rv = 1;
	size_t			r;
	char			b[XML_PAGE_SIZE];

	if (xl == NULL)
		return (1);

	TAILQ_INIT(xl);

	bzero(&ctx, sizeof ctx);
	ctx.depth = -1;

	xml = XML_ParserCreate(NULL);
	if (xml == NULL)
		return (1);

	ctx.xml_parser = xml;
	ctx.xml_el = xl;

	XML_SetUserData(xml, &ctx);
	XML_SetElementHandler(xml, xmlsd_start, xmlsd_end);
	XML_SetCharacterDataHandler(xml, xmlsd_chardata);

	for (done = 0; done == 0;) {
		r = fread(b, 1, sizeof b, f);
		if (ferror(f))
			goto done;
		done = feof(f);
		if (XML_Parse(xml, b, r, done) == XML_STATUS_ERROR)
			goto done; /* XXX do return codes */
	}

	XML_ParserFree(xml);
	rv = 0;
done:
	return (rv);
}

void
xmlsd_unwind(struct xmlsd_element_list *xl)
{
	struct xmlsd_element		*xe;
	struct xmlsd_attribute		*xa;

	while ((xe = TAILQ_FIRST(xl))) {
		TAILQ_REMOVE(xl, xe, entry);

		/* free attributes */
		while ((xa = TAILQ_FIRST(&xe->attr_list))) {
			TAILQ_REMOVE(&xe->attr_list, xa, entry);
			if (xa->name)
				free(xa->name);
			if (xa->value)
				free(xa->value);
		}

		/* free element */
		if (xe->name)
			free(xe->name);
		if (xe->value)
			free(xe->value);
		free(xe);
	}
}

#if 0
int
main(int argc, char *argv[])
{
	struct xmlsd_element_list	xl;
	struct xmlsd_element		*xe;
	struct xmlsd_attribute		*xa;

	xml_parse(stdin, &xl);
	TAILQ_FOREACH(xe, &xl, entry) {
		fprintf(stderr, "%d %s = %s (parent = %s)\n",
		    xe->depth,
		    xe->name,
		    xe->value ? xe->value : "NOVAL",
		    xe->parent ? xe->parent->name : "NOPARENT");
		TAILQ_FOREACH(xa, &xe->attr_list, entry)
			fprintf(stderr, "\t%s = %s\n", xa->name, xa->value);
	}

	xml_unwind(&xl);

	return (0);
}
#endif
