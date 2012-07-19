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
#include <pthread.h>

#define XMLSD_MEM_MAXSIZE	(10 * 1024 * 1024)

extern char			*__progname;

int				verbose;
int				completed;
int				created;
pthread_mutex_t			mtx;

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

void *
thread_it(void *p)
{
	struct xmlsd_document		*xd;
	struct xmlsd_element		*xe;
	int				f;
	char				*b, *filename = (char *)p;
	struct stat			sb;

	if (xmlsd_doc_alloc(&xd) != XMLSD_ERR_SUCCES)
		errx(1,"xmlsd_doc_alloc");

	f = open(filename, O_RDONLY, 0);
	if (f == -1)
		err(1, "open");
	if (fstat(f, &sb) == -1)
		err(1, "stat");
	if (sb.st_size > XMLSD_MEM_MAXSIZE)
		errx(1, "file too big");
	b = malloc(sb.st_size);
	if (b == NULL)
		err(1, "malloc");
	memset(b, 0, sb.st_size);
	if (read(f, b, sb.st_size) != sb.st_size)
		err(1, "read");

	if (xmlsd_parse_mem(b, sb.st_size, xd) != XMLSD_ERR_SUCCES)
		errx(1, "xmlsd_parse");
	if (verbose) {
		XMLSD_DOC_FOREACH_ELEM(xe, xd) {
			print_element(xe);
		}
	}

	xmlsd_doc_free(xd);

	free(b);
	close(f);
	f = 0;

	pthread_mutex_lock(&mtx);
	completed++;
	pthread_mutex_unlock(&mtx);

	return (NULL);
}

#define MAX_THREADS	(250000)

int
main(int argc, char *argv[])
{
	pthread_t	thr[MAX_THREADS];
	int		i, r;

	if (argc != 2)
		errx(1, "usage %s <filename>", __progname);

	pthread_mutex_init(&mtx, NULL);

	memset(thr, 0, sizeof(thr));
	for (i = 0; i < MAX_THREADS;) {
		if (pthread_create(&thr[i], NULL, thread_it, (void *)argv[1]) == NULL) {
			/* thread create stalled, just try again */
			continue;
		}
		created++;
		i++;
	}

	for (i = 0; i < MAX_THREADS; i++) {
		if ((r = pthread_join(thr[i], NULL)))
			printf("i %d e %d\n", i, r);
	}

	printf("tried: %d\ncreated: %d\ncompleted: %d\n", i, created, completed);

	if (completed != i)
		errx(1, "TEST FAILED");

	return (0);
}
