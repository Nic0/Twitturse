#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <curl/curl.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "utils.h"
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
