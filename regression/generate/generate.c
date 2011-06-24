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
	struct xmlsd_element_list	xl;
	struct xmlsd_element		*xe, *top_xe, *xe1;
	char				*str;
	FILE				*fp;

	TAILQ_INIT(&xl);

	fp = fopen("example.xml", "r");
	if (fp == NULL)
		errx(1, "unable to open input file");

	if (xmlsd_parse_file(fp, &xl) != XMLSD_ERR_SUCCES)
		errx(1, "xmlsd_parse");
	fclose(fp);

	str = xmlsd_generate(&xl, malloc);

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

	xmlsd_unwind(&xl);

	TAILQ_INIT(&xl);

	top_xe = xmlsd_create(&xl, "level0");
	xmlsd_set_attr(top_xe, "version", "1");

	xe = xmld_add_element(&xl, top_xe, "level1a");
	xmlsd_set_attr(xe, "l1a_attr", "l1a");
	xe = xmld_add_element(&xl, top_xe, "level1b");
	xmlsd_set_attr(xe, "l1b_attr", "l1b");
	xe = xmld_add_element(&xl, top_xe, "level1c");
	xmlsd_set_attr(xe, "l1c_attr", "l1c");
	xmlsd_set_value(xe, "something");
	xe1 = xmld_add_element(&xl, top_xe, "level1d");
	xmlsd_set_attr(xe1, "l1d_attr", "l1d");
	xe = xmld_add_element(&xl, xe1, "level2");
	xmlsd_set_attr(xe, "name", "a");
	xmlsd_set_attr(xe, "l2a_attr0", "0");
	xmlsd_set_attr(xe, "l2a_attrf", "1");
	xe = xmld_add_element(&xl, xe1, "level2");
	xmlsd_set_attr(xe, "name", "b");
	xmlsd_set_attr(xe, "l2b_attr0", "0");
	xe = xmld_add_element(&xl, xe, "level3");
	xe = xmld_add_element(&xl, xe, "level4");
	xe = xmld_add_element(&xl, xe, "level5");
	xe = xmld_add_element(&xl, xe, "level6");
	xe = xmld_add_element(&xl, xe, "level7");
	xe = xmld_add_element(&xl, xe, "level8");
	xe = xmld_add_element(&xl, xe, "level9");
	xe = xmld_add_element(&xl, xe, "level10");
	xe = xmld_add_element(&xl, xe, "level11");
	xmlsd_set_value(xe, "foo");
	xe = xmld_add_element(&xl, xe1, "level2");
	xmlsd_set_attr(xe, "name", "c");
	xmlsd_set_attr(xe, "l2c_attr0", "0");
	xe = xmld_add_element(&xl, xe, "level3b");
	xmlsd_set_attr(xe, "name", "l3b");

	str = xmlsd_generate(&xl, malloc);
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

	return (0);
}
