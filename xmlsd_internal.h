struct xmlsd_attribute {
	TAILQ_ENTRY(xmlsd_attribute)	entry;
	char				*name;
	char				*value;
};
TAILQ_HEAD(xmlsd_attribute_list, xmlsd_attribute);

TAILQ_HEAD(xmlsd_element_list, xmlsd_element);
struct xmlsd_element {
	TAILQ_ENTRY(xmlsd_element)	 entry;
	struct xmlsd_attribute_list	 attr_list;
	struct xmlsd_element_list	 children;
	struct xmlsd_element		*parent;
	char				*name;
	char				*value;
	int				 depth;
};

struct xmlsd_document {
	struct xmlsd_element_list	 children;
};
