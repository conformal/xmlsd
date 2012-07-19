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

/*
 * NOTE: the output from xml_generate differs from the input 'a' file
 * because the generator code will always normalize tags that
 * have no children or values into <foo ... /> instead of <foo ...></foo>
 */

int
main(int argc, char *argv[])
{
	struct xmlsd_document		*xd;
	struct xmlsd_element		*xe, *top_xe, *xe1;
	char				*str;
	FILE				*fp;
	size_t				 sz;

	if (xmlsd_doc_alloc(&xd) != XMLSD_ERR_SUCCES)
		errx(1,"xmlsd_doc_alloc");

	fp = fopen("example.xml", "r");
	if (fp == NULL)
		errx(1, "unable to open input file");

	if (xmlsd_parse_file(fp, xd) != XMLSD_ERR_SUCCES)
		errx(1, "xmlsd_parse");
	fclose(fp);

	str = xmlsd_generate(xd, malloc, &sz, XMLSD_GEN_ADD_HEADER);

	if (str)
		printf("%s", str);
	else
		printf("NULL str returned");
	fp = fopen("output1.xml", "w");
	if (fp == NULL)
		errx(1, "fail to write output file c");
	fwrite(str, strlen(str), sizeof *str, fp);
	fclose(fp);
	free (str);

	xmlsd_doc_clear(xd);

	top_xe = xmlsd_doc_add_elem(xd, NULL, "level0");
	xmlsd_elem_set_attr(top_xe, "version", "1");

	xe = xmlsd_doc_add_elem(xd, top_xe, "level1a");
	xmlsd_elem_set_attr(xe, "l1a_attr", "l1a");
	xe = xmlsd_doc_add_elem(xd, top_xe, "level1b");
	xmlsd_elem_set_attr(xe, "l1b_attr", "l1b");
	xe = xmlsd_doc_add_elem(xd, top_xe, "level1c");
	xmlsd_elem_set_attr(xe, "l1c_attr", "l1c");
	xmlsd_elem_set_value(xe, "something");
	xe1 = xmlsd_doc_add_elem(xd, top_xe, "level1d");
	xmlsd_elem_set_attr(xe1, "l1d_attr", "l1d");
	xe = xmlsd_doc_add_elem(xd, xe1, "level2");
	xmlsd_elem_set_attr(xe, "name", "a");
	xmlsd_elem_set_attr(xe, "l2a_attr0", "0");
	xmlsd_elem_set_attr(xe, "l2a_attrf", "1");
	xe = xmlsd_doc_add_elem(xd, xe1, "level2");
	xmlsd_elem_set_attr(xe, "name", "b");
	xmlsd_elem_set_attr(xe, "l2b_attr0", "0");
	xe = xmlsd_doc_add_elem(xd, xe, "level3");
	xe = xmlsd_doc_add_elem(xd, xe, "level4");
	xe = xmlsd_doc_add_elem(xd, xe, "level5");
	xe = xmlsd_doc_add_elem(xd, xe, "level6");
	xe = xmlsd_doc_add_elem(xd, xe, "level7");
	xe = xmlsd_doc_add_elem(xd, xe, "level8");
	xe = xmlsd_doc_add_elem(xd, xe, "level9");
	xe = xmlsd_doc_add_elem(xd, xe, "level10");
	xe = xmlsd_doc_add_elem(xd, xe, "level11");
	xmlsd_elem_set_value(xe, "foo");
	xe = xmlsd_doc_add_elem(xd, xe1, "level2");
	xmlsd_elem_set_attr(xe, "name", "c");
	xmlsd_elem_set_attr(xe, "l2c_attr0", "0");
	xe = xmlsd_doc_add_elem(xd, xe, "level3b");
	xmlsd_elem_set_attr(xe, "name", "l3b");

	str = xmlsd_generate(xd, malloc, &sz, XMLSD_GEN_ADD_HEADER);
	if (str)
		printf("%s", str);
	else
		printf("NULL str returned");
	fp = fopen("output2.xml", "w");
	if (fp == NULL)
		errx(1, "fail to write output file c");
	fwrite(str, strlen(str), sizeof *str, fp);
	fclose(fp);
	free (str);
	xmlsd_doc_free(xd);

	return (0);
}
