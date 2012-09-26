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

/* NULL cmd */
struct xmlsd_v_elements filesystem_no_cmd[] = {
	{ "filesystem", vfilesystem },
	{ "nocmdhere", NULL },
	{ NULL, NULL },
};

/* path for first element */
struct xmlsd_v_elem	vfilesystem_root_path[] = {
	{ "filesystem", "filesystem", filesystem_attr },
	{ "dir", "dir.filesystem", dir_attr },
	{ NULL, NULL, NULL },
};

struct xmlsd_v_elements filesystem_root_path[] = {
	{ "filesystem", vfilesystem_root_path },
	{ NULL, NULL },
};

/* NULL path for later element */
struct xmlsd_v_elem	vfilesystem_no_path[] = {
	{ "filesystem", "", filesystem_attr },
	{ "dir", NULL, dir_attr },
	{ NULL, NULL, NULL },
};

struct xmlsd_v_elements filesystem_no_path[] = {
	{ "filesystem", vfilesystem_no_path },
	{ NULL, NULL },
};

/* invalid path (unimplemented...) */
struct xmlsd_v_elem	vfilesystem_invalid_path[] = {
	{ "filesystem", "", filesystem_attr },
	{ "dir", "", dir_attr },
	{ NULL, NULL, NULL },
};

struct xmlsd_v_elements filesystem_invalid_path[] = {
	{ "filesystem", vfilesystem_invalid_path },
	{ NULL, NULL },
};
/* attr with funny flags */
struct xmlsd_v_attr	filesystem_attr_unrecognised_flag[] = {
	{ "version", 0xfffffff },
	{ NULL }
};

struct xmlsd_v_elem	vfilesystem_unrecognised_attr_flag[] = {
	{ "filesystem", "", filesystem_attr_unrecognised_flag },
	{ "dir", "dir.filesystem", dir_attr },
	{ NULL, NULL, NULL },
};

struct xmlsd_v_elements filesystem_unrecognised_attr_flag[] = {
	{ "filesystem", vfilesystem_unrecognised_attr_flag },
	{ NULL, NULL },
};
/* negative min_occurs */ 
struct xmlsd_v_elem	vfilesystem_min_occurs_negative[] = {
	{ "filesystem", "", filesystem_attr, -1 },
	{ "dir", "dir.filesystem", dir_attr },
	{ NULL, NULL, NULL },
};

struct xmlsd_v_elements filesystem_min_occurs_negative[] = {
	{ "filesystem", vfilesystem_min_occurs_negative },
	{ NULL, NULL },
};
/* negative max_occurs */ 
struct xmlsd_v_elem	vfilesystem_max_occurs_negative[] = {
	{ "filesystem", "", filesystem_attr, 0, -1 },
	{ "dir", "dir.filesystem", dir_attr },
	{ NULL, NULL, NULL },
};

struct xmlsd_v_elements filesystem_max_occurs_negative[] = {
	{ "filesystem", vfilesystem_max_occurs_negative },
	{ NULL, NULL },
};
/* min > max occurs */
struct xmlsd_v_elem	vfilesystem_max_less_than_min[] = {
	{ "filesystem", "", filesystem_attr, 5, 3 },
	{ "dir", "dir.filesystem", dir_attr },
	{ NULL, NULL, NULL },
};

struct xmlsd_v_elements filesystem_max_less_than_min[] = {
	{ "filesystem", vfilesystem_max_less_than_min },
	{ NULL, NULL },
};

void
check_validate(struct xmlsd_v_elements *cmd,
    enum xmlsd_validate_v_elements_failure expected_result, const char *name)
{
	struct xmlsd_v_elements_validation	 xvev;
	enum xmlsd_validate_v_elements_failure 	 ret;
	char				 	*warning;

	bzero(&xvev, sizeof(xvev));
	if ((ret = xmlsd_validate_v_elements(cmd, &xvev)) !=
	    expected_result) {
		printf("%s FAILURE: unexpected result %d, expected %d\n",
		    name, ret, expected_result);
		return;
	}

	warning = xmlsd_get_validate_v_elements_failure_string(&xvev);
	if (warning == NULL) {
		printf("%s: FAILURE out of memory for error message", name);
		return;
	}

	printf("%s: PASS! error message %s\n", name, warning);
	free(warning);
}

int
main(int argc, char *argv[])
{
	check_validate(filesystem_working, XMLSD_VALIDATE_ELEMENTS_NO_ERROR,
	    "working");
	check_validate(filesystem_no_cmd, XMLSD_VALIDATE_ELEMENTS_NO_CMD,
	    "no cmd");
	check_validate(filesystem_root_path, XMLSD_VALIDATE_ELEMENTS_ROOT_PATH,
	    "root path");
	check_validate(filesystem_no_path, XMLSD_VALIDATE_ELEMENTS_NO_PATH,
	    "no path");
	check_validate(filesystem_invalid_path,
	    XMLSD_VALIDATE_ELEMENTS_INVALID_PATH, "invalid path");
	check_validate(filesystem_unrecognised_attr_flag,
	    XMLSD_VALIDATE_ELEMENTS_UNRECOGNISED_ATTR_FLAG,
	    "unrecognised attr");
	check_validate(filesystem_min_occurs_negative,
	    XMLSD_VALIDATE_ELEMENTS_MIN_OCCURS_NEGATIVE, "negative min");
	check_validate(filesystem_max_occurs_negative,
	    XMLSD_VALIDATE_ELEMENTS_MAX_OCCURS_NEGATIVE, "negative max");
	check_validate(filesystem_max_less_than_min,
	    XMLSD_VALIDATE_ELEMENTS_MAX_LESS_THAN_MIN, "max less than min");

	return (0);
}
