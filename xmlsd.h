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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <expat.h>

#include <sys/queue.h>

#define XMLSD_ERR_UNKNOWN	(-1)
#define XMLSD_ERR_SUCCES	(0)
#define XMLSD_ERR_PARSER	(1)
#define XMLSD_ERR_RESOURCE	(2)
#define XMLSD_ERR_EXTERNAL	(3)
#define XMLSD_ERR_OVERFLOW	(4)
#define XMLSD_ERR_INTEGRITY	(5)

struct xmlsd_attribute {
	TAILQ_ENTRY(xmlsd_attribute)	entry;
	char				*name;
	char				*value;
};
TAILQ_HEAD(xmlsd_attribute_list, xmlsd_attribute);

struct xmlsd_element {
	TAILQ_ENTRY(xmlsd_element)	entry;
	struct xmlsd_attribute_list	attr_list;
	struct xmlsd_element		*parent;
	char				*name;
	char				*value;
	int				depth;
};
TAILQ_HEAD(xmlsd_element_list, xmlsd_element);

int			xmlsd_unwind(struct xmlsd_element_list *);
int			xmlsd_parse_file(FILE *, struct xmlsd_element_list *);
int			xmlsd_parse_mem(char *, size_t,
			    struct xmlsd_element_list *);
