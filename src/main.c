#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "curl.h"

#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))
/*
typedef struct status_t
{
    char    *pseudo;
    char    *text;
    struct   status_t *next;
    struct   status_t *prev;
} status_t;

status_t * 
initStatus (status_t *status)
{
    status->pseudo = NULL;
    status->text = NULL;
    status->next = (maloc sizeof(status_t));
    status->prev = (maloc sizeof(status_t));
}*/

static void print_element_names (xmlNode *a_node)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    
        if (cur_node->parent != NULL) { 
            if (cur_node->type == XML_TEXT_NODE 
                 && strcmp(cur_node->parent->name, "screen_name") == 0)
                printf ("<%s>\t", cur_node->content);

            if (cur_node->type == XML_TEXT_NODE 
                 && strcmp(cur_node->parent->name, "text") == 0)
                printf ("%s\n", cur_node->content);
        }
        print_element_names(cur_node->children);
    }
}

int main (void)
{
    char *data;
    data = get_URL ("https://api.twitter.com/statuses/home_timeline.xml");
    //data = get_URL ("http://search.twitter.com/search.atom?q=%23Archlinux");

    xmlDocPtr xmldoc;
    xmlNode *xmlroot = NULL;

    if ((xmldoc = xmlParseMemory (data, strlen(data))) == NULL) {
        ERROR;
        return EXIT_FAILURE;
    }

    if ((xmlroot = xmlDocGetRootElement (xmldoc)) == NULL) {
        fprintf (stderr, "Le document est vide\n");
        xmlFreeDoc (xmldoc);
        return EXIT_FAILURE;
    }

    print_element_names(xmlroot);
    
    xmlFreeDoc (xmldoc);
    xmlCleanupParser();
    return EXIT_SUCCESS;
}
