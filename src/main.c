/***********************************************************
*
*       Twitturse   v 0.0.3
*
*       Nic0 <nicolas.caen (at) gmail.com>
*       03/05/2010
*
*       Software distributed under GPL licence 
*
***********************************************************/

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

typedef struct status_t
{
    unsigned long    id;
    xmlChar         *pseudo;
    xmlChar         *text;
    struct status_t *next;
    struct status_t *prev;
} status_t;

typedef struct statuses_t
{
    status_t    *first;
    status_t    *last;
    int          count;
} statuses_t;

status_t * 
initStatus (status_t *status)
{
    if ((status = malloc (sizeof(status_t))) == NULL)
        return NULL;
    status->id      = 0;
    status->pseudo  = NULL;
    status->text    = NULL;
    status->next    = NULL;
    status->prev    = NULL;
    return status;
}

statuses_t *
initStatuses (statuses_t *statuses)
{
    if ((statuses = malloc (sizeof(statuses_t))) == NULL)
        return NULL;
    statuses->count = 0;
    statuses->first = NULL;
    statuses->last  = NULL;
    return statuses;
}

static void 
getStatuses (xmlNode *a_node, statuses_t *statuses, status_t *cur_status)
{
    xmlNode     *cur_node   = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {

            if (cur_node->type == XML_TEXT_NODE 
                 && strcmp(cur_node->parent->name, "text") == 0) {

                    cur_status->text = cur_node->content;
            }

            if (strcmp(cur_node->name, "screen_name") == 0) {
                cur_status->pseudo = cur_node->children->content;

                status_t *node_status;
                if ((node_status = initStatus(node_status)) == NULL)
                    ERROR;
                *node_status = *cur_status;

                if (statuses->count == 0) {
                    statuses->last = node_status;
                    statuses->first = node_status;
                } else {
                    statuses->last->next = node_status;
                    node_status->prev = statuses->last;
                    statuses->last = node_status;
                }
                cur_status = initStatus(cur_status);
                statuses->count++;
            }

        getStatuses(cur_node->children, statuses, cur_status);
    }
}

void
printStatuses (statuses_t *statuses)
{
    status_t *status = NULL;
    status = statuses->last;
    while (1) {
        printf("<%s>\t%s\n", status->pseudo, status->text);
        if (status->prev != NULL)
            status = status->prev;
        else
            break;
    }
}

int 
main (void)
{
    static char *data       = NULL;
    xmlDoc      *xmldoc     = NULL;
    xmlNode     *xmlroot    = NULL;
    statuses_t  *statuses   = NULL;

    data = get_URL ("https://api.twitter.com/statuses/home_timeline.xml");

    if ((xmldoc = xmlParseMemory (data, strlen(data))) == NULL) {
        ERROR;
        return EXIT_FAILURE;
    }

    if ((xmlroot = xmlDocGetRootElement (xmldoc)) == NULL) {
        fprintf (stderr, "Le document est vide\n");
        xmlFreeDoc (xmldoc);
        return EXIT_FAILURE;
    }

    if ((statuses = initStatuses(statuses)) == NULL) {
        ERROR;
        return EXIT_FAILURE;
    }
    
    status_t *cur_status = NULL;
    cur_status = initStatus(cur_status);
    getStatuses(xmlroot, statuses, cur_status);

    printStatuses(statuses);
    
    xmlFreeDoc (xmldoc);
    xmlCleanupParser();
    return EXIT_SUCCESS;
}
