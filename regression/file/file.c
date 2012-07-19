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

#include "../../xmlsd.h"

#include <err.h>
#include <string.h>

static void
print_element(struct xmlsd_element *xe)
{
	struct xmlsd_element		*xc;
	struct xmlsd_attribute		*xa;

	fprintf(stderr, "%d %s = %s (parent = %s)\n",
	    xmlsd_elem_get_depth(xe),
	    xmlsd_elem_get_name(xe),
	    xmlsd_elem_get_value(xe) ? xmlsd_elem_get_value(xe) : "NOVAL",
	    xmlsd_elem_get_parent(xe) ?
	        xmlsd_elem_get_name(xmlsd_elem_get_parent(xe)) :"NOPARENT");
	XMLSD_ELEM_FOREACH_ATTR(xa, xe)
		fprintf(stderr, "\t%s = %s\n", xmlsd_attr_get_name(xa),
		    xmlsd_attr_get_value(xa));
	XMLSD_ELEM_FOREACH_CHILDREN(xc, xe)
		print_element(xc);
}
int
main(int argc, char *argv[])
{
	struct xmlsd_document		*xd;
	struct xmlsd_element		*xe;

	if (xmlsd_doc_alloc(&xd) != XMLSD_ERR_SUCCES)
		errx(1,"xmlsd_doc_alloc");

	if (xmlsd_parse_file(stdin, xd) != XMLSD_ERR_SUCCES)
		errx(1, "xmlsd_parse");
	XMLSD_DOC_FOREACH_ELEM(xe, xd) {
		print_element(xe);
	}

	xmlsd_doc_free(xd);

	return (0);
}
