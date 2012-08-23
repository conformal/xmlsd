/*
 * Copyright (c) 2012 Conformal Systems LLC <info@conformal.com>
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

/* Valid validation structures */
struct xmlsd_v_attr	file_attr[] = {
	{ "version" },
	{ "name" },
	{ NULL }
};

struct xmlsd_v_attr	dir_attr[] = {
	{ "version" },
	{ "name" },
	{ NULL }
};

struct xmlsd_v_attr	filesystem_attr[] = {
	{ "version" },
	{ NULL }
};

struct xmlsd_v_elem	vfilesystem[] = {
	{ "filesystem", "", filesystem_attr },
	{ "dir", "dir.filesystem", dir_attr },
	{ "file", "file.dir.filesystem", file_attr },
	{ NULL, NULL, NULL },
};

struct xmlsd_v_elements filesystem_working[] = {
	{ "filesystem", vfilesystem },
	{ NULL, NULL },
};

/* unrecognised element */
struct xmlsd_v_elem	vfilesystem_unrecognised_element[] = {
	{ "filesystem", "", filesystem_attr },
	{ "dir", "dir.filesystem", dir_attr },
	{ NULL, NULL, NULL },
};

struct xmlsd_v_elements filesystem_unrecognised_element[] = {
	{ "filesystem", vfilesystem_unrecognised_element },
	{ NULL, NULL },
};

/* unrecognised attribute */
struct xmlsd_v_attr	bad_file_attr[] = {
	{ "version" },
	{ NULL }
};
struct xmlsd_v_elem	vfilesystem_unrecognised_attribute[] = {
	{ "filesystem", "", filesystem_attr },
	{ "dir", "dir.filesystem", dir_attr },
	{ "file", "file.dir.filesystem", bad_file_attr },
	{ NULL, NULL, NULL },

};

struct xmlsd_v_elements filesystem_unrecognised_attribute[] = {
	{ "filesystem", vfilesystem_unrecognised_attribute },
	{ NULL, NULL },
};

/* too many occurences */
struct xmlsd_v_elem	vfilesystem_too_many_occurrences[] = {
	{ "filesystem", "", filesystem_attr },
	{ "dir", "dir.filesystem", dir_attr },
	{ "file", "file.dir.filesystem", file_attr, 0, 3 },
	{ NULL, NULL, NULL },

};

struct xmlsd_v_elements filesystem_too_many_occurrences[] = {
	{ "filesystem", vfilesystem_too_many_occurrences },
	{ NULL, NULL },
};

/* too few occurences */
struct xmlsd_v_elem	vfilesystem_too_few_occurrences[] = {
	{ "filesystem", "", filesystem_attr },
	{ "dir", "dir.filesystem", dir_attr },
	{ "file", "file.dir.filesystem", file_attr, 2, 4 },
	{ NULL, NULL, NULL },

};

struct xmlsd_v_elements filesystem_too_few_occurrences[] = {
	{ "filesystem", vfilesystem_too_few_occurrences },
	{ NULL, NULL },
};

/* missing required attr */
struct xmlsd_v_attr	dir_required_attr[] = {
	{ "version", XMLSD_V_ATTR_F_REQUIRED },
	{ "name", XMLSD_V_ATTR_F_REQUIRED },
	{ NULL }
};

struct xmlsd_v_elem	vfilesystem_missing_required_attr[] = {
	{ "filesystem", "", filesystem_attr },
	{ "dir", "dir.filesystem", dir_required_attr },
	{ "file", "file.dir.filesystem", file_attr },
	{ NULL, NULL, NULL },
};

struct xmlsd_v_elements filesystem_missing_required_attr[] = {
	{ "filesystem", vfilesystem_missing_required_attr },
	{ NULL, NULL },
};

/* unrecognised command */
struct xmlsd_v_elements filesystem_unrecognised_command[] = {
	{ "filesystemp", vfilesystem }, /* typo intentional */
	{ NULL, NULL },
};

/* empty xml */

/*
 * The following two are currently not tested.
 *
 * one is an internal constraint.
 * the other is really an assert and abort condition and can not happen with use
 * of the api..
 */
/* path too long... (deeply nested) */
/* root has parent */

void
check_validate(struct xmlsd_document *xd, struct xmlsd_v_elements *cmd,
    enum xmlsd_validate_reason expected_result, const char *name)
{
	struct xmlsd_validate_failure	  xvf;
	enum xmlsd_validate_reason	  ret;
	char				 *warning;

	if ((ret = xmlsd_validate_info(xd, cmd, &xvf)) !=
	    expected_result) {
		printf("%s FAILURE: unexpected result %d, expected %d\n",
		    name, ret, expected_result);
		return;
	}

	warning = xmlsd_get_validate_failure_string(&xvf);

	printf("%s: PASS! error message %s\n", name, warning);
	free(warning);
}

int
main(int argc, char *argv[])
{
	struct xmlsd_document		*xd;
	FILE				*fp;

	if (xmlsd_doc_alloc(&xd) != XMLSD_ERR_SUCCES)
		errx(1,"xmlsd_doc_alloc");

	fp = fopen("example.xml", "r");
	if (fp == NULL)
		errx(1, "unable to open input file");

	if (xmlsd_parse_file(fp, xd) != XMLSD_ERR_SUCCES)
		errx(1, "xmlsd_parse");
	fclose(fp);

	check_validate(xd, filesystem_working, XMLSD_VALIDATE_NO_ERROR,
	    "working");
	check_validate(xd, filesystem_unrecognised_element,
	    XMLSD_VALIDATE_UNRECOGNISED_ELEMENT, "unrecognised element");
	check_validate(xd, filesystem_unrecognised_attribute,
	    XMLSD_VALIDATE_UNRECOGNISED_ATTRIBUTE, "unrecognised attribute");
	check_validate(xd, filesystem_too_many_occurrences,
	    XMLSD_VALIDATE_TOO_MANY_OCCURRENCES, "too many occurrences");
	check_validate(xd, filesystem_too_few_occurrences, 
	    XMLSD_VALIDATE_TOO_FEW_OCCURRENCES, "too few occurrences");
	check_validate(xd, filesystem_missing_required_attr,
	    XMLSD_VALIDATE_MISSING_REQUIRED_ATTRIBUTE, "missing required attr");
	check_validate(xd, filesystem_unrecognised_command,
	    XMLSD_VALIDATE_UNRECOGNISED_COMMAND, "unrecognised command");

	xmlsd_doc_clear(xd);

	check_validate(xd, filesystem_working, XMLSD_VALIDATE_EMPTY_XML,
	    "empty xml");

	xmlsd_doc_free(xd);

	return (0);
}
