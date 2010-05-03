#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "xml.h"


#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))

static void
print_element_names(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;


    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    
        if (cur_node->parent != NULL) { 
            if (cur_node->type == XML_TEXT_NODE 
                 && strcmp(cur_node->parent->name, "screen_name") == 0) {
                printf ("<%s>\t", cur_node->content);
            }
            
            if (cur_node->type == XML_TEXT_NODE 
                 && strcmp(cur_node->parent->name, "text") == 0) {
                printf ("%s\n", cur_node->content);
            }
        }

        print_element_names(cur_node->children);
    }
}
