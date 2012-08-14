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

#include "xmlsd.h"
#include "xmlsd_internal.h"

#include <stdlib.h>
#include <string.h>

/*
 * Allocate a new xmlsd_document into `xdp'.
 *
 * Returns an error code on failure, in which case xdp is undefined.
 */
int
xmlsd_doc_alloc(struct xmlsd_document **xdp)
{
	struct xmlsd_document *xd;

	xd = calloc(1, sizeof(*xd));
	if (xd == NULL)
		return (XMLSD_ERR_RESOURCE);

	xd->root = NULL;
	*xdp = xd;
	return (XMLSD_ERR_SUCCES);
}

/*
 * Empty out and free all entries in `xd' but leave the document itself
 * allocated.
 */
void
xmlsd_doc_clear(struct xmlsd_document *xd)
{
	struct xmlsd_element *xe;

	if (xd == NULL)
		return;

	/* Recursively empty tree */
	while ((xe = xmlsd_doc_get_first_elem(xd)) != NULL) {
		xmlsd_doc_remove_elem(xd, xe);
	}
}

/*
 * Free the document ``xd'' and all its entries.
 */
void
xmlsd_doc_free(struct xmlsd_document *xd)
{
	if (xd == NULL)
		return;
	xmlsd_doc_clear(xd);
	free (xd);
}

/*
 * Add an element with name `name' to `xd' with parent `xe'.
 * If xe is NULL then add element at the top level.
 */
struct xmlsd_element *
xmlsd_doc_add_elem(struct xmlsd_document *xd, struct xmlsd_element *xe,
    const char *name)
{
	struct xmlsd_element *nxe = NULL;

	if (xd == NULL || name == NULL || (strlen(name) == 0))
		goto fail;

	/* XXX xmlsd_elem_alloc(parent, name)? */
        nxe = calloc(1, sizeof *nxe);
        if (nxe == NULL)
		goto fail;

	nxe->name = strdup(name);
	if (nxe->name == NULL)
		goto fail;

        TAILQ_INIT(&nxe->attr_list);
	TAILQ_INIT(&nxe->children);
	nxe->depth = xe ? xe->depth + 1 : 0;
	nxe->parent = xe;

	if (xe)
		TAILQ_INSERT_TAIL(&xe->children, nxe, entry);
	else {
		if (xd->root != NULL)
			goto fail;
		xd->root = nxe;
	}

	return nxe;

fail:
	if (nxe) {
		if (nxe->name)
			free(nxe->name);
		if (nxe)
			free(nxe);
	}
	return NULL;
}

/*
 * Remove elem and its children from xd.
 */
void
xmlsd_doc_remove_elem(struct xmlsd_document *xd, struct xmlsd_element *xe)
{
	struct xmlsd_element	*xc;

	if (xe == NULL || xd == NULL || xd->root == NULL)
		return;

	while ((xc = xmlsd_elem_get_first_child(xe)) != NULL)
		xmlsd_doc_remove_elem(xd, xc);
	if (xe->parent)
		TAILQ_REMOVE(&xe->parent->children, xe, entry);
	else {
		xd->root = NULL;
	}
	xmlsd_elem_free(xe);
}
/*
 * Return boolean whether or not xd is an empty document.
 */
int
xmlsd_doc_is_empty(struct xmlsd_document *xd)
{
	return (xd->root == NULL);
}

/*
 * Get the first top-level element in xd NULL if it is empty.
 */
struct xmlsd_element	*
xmlsd_doc_get_first_elem(struct xmlsd_document *xd)
{
	return (xd->root);
}
