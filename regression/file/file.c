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

#include "../xmlsd.h"

int
main(int argc, char *argv[])
{
	struct xmlsd_element_list	xl;
	struct xmlsd_element		*xe;
	struct xmlsd_attribute		*xa;

	TAILQ_INIT(&xl);

	if (xmlsd_parse_file(stdin, &xl) != XMLSD_ERR_SUCCES)
		errx(1, "xmlsd_parse");
	TAILQ_FOREACH(xe, &xl, entry) {
		fprintf(stderr, "%d %s = %s (parent = %s)\n",
		    xe->depth,
		    xe->name,
		    xe->value ? xe->value : "NOVAL",
		    xe->parent ? xe->parent->name : "NOPARENT");
		TAILQ_FOREACH(xa, &xe->attr_list, entry)
			fprintf(stderr, "\t%s = %s\n", xa->name, xa->value);
	}

	xmlsd_unwind(&xl);

	return (0);
}
