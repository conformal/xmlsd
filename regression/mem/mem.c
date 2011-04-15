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

#include "../../xmlsd.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>

#define XMLSD_MEM_MAXSIZE	(10 * 1024 * 1024)

extern char			*__progname;

int
main(int argc, char *argv[])
{
	struct xmlsd_element_list	xl;
	struct xmlsd_element		*xe;
	struct xmlsd_attribute		*xa;
	int				f;
	char				*b;
	struct stat			sb;

	TAILQ_INIT(&xl);

	if (argc != 2)
		errx(1, "usage %s <filename>", __progname);
	f = open(argv[1], O_RDONLY, 0);
	if (f == -1)
		err(1, "open");
	if (fstat(f, &sb) == -1)
		err(1, "stat");
	if (sb.st_size > XMLSD_MEM_MAXSIZE)
		errx(1, "file too big");
	b = malloc(sb.st_size);
	if (b == NULL)
		err(1, "malloc");
	if (read(f, b, sb.st_size) != sb.st_size)
		err(1, "read");

	if (xmlsd_parse_mem(b, sb.st_size, &xl) != XMLSD_ERR_SUCCES)
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

	free(b);
	close(f);
	f = NULL;

	return (0);
}
