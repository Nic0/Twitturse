/***********************************************************
*
*       Twitturse   v 0.0.5
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
#include <unistd.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

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

statuses_t * 
getNewStatuses (xmlDoc *xmldoc, statuses_t *statuses, statuses_t *tmp_statuses)
{
    xmlXPathContextPtr xpathCtx = NULL;
    
    xmlXPathObjectPtr pseudo = NULL;
    xmlXPathObjectPtr text = NULL;

    xpathCtx = xmlXPathNewContext(xmldoc);
    if (xmldoc == NULL) {
        ERROR;
        return NULL;
    }

    text = xmlXPathEvalExpression("/statuses/status/text/text()", xpathCtx);
    if (text == NULL) {
        ERROR;
        return NULL;
    }

    pseudo = xmlXPathEvalExpression("/statuses/status/user/screen_name/text()", xpathCtx);
    if (pseudo == NULL) {
        ERROR;
        return NULL;
    }

    int size = 0;
    int i;

    size = text->nodesetval ? text->nodesetval->nodeNr : 0;

    for (i = 0; i < size; ++i) {
        //printf ("<%s>\t", pseudo->nodesetval->nodeTab[i]->content);
        //printf ("%s\n", text->nodesetval->nodeTab[i]->content);
        
        if(statuses->count != 0)
            if(text->nodesetval->nodeTab[i]->content != statuses->first->text)
                return tmp_statuses;

                status_t *cur_status = NULL;
                cur_status = initStatus(cur_status);

                cur_status->text = text->nodesetval->nodeTab[i]->content;
                cur_status->pseudo = pseudo->nodesetval->nodeTab[i]->content;

                if (tmp_statuses->count == 0) {
                    tmp_statuses->last = cur_status;
                    tmp_statuses->first = cur_status;
                } else {
                    tmp_statuses->last->next = cur_status;
                    cur_status->prev = tmp_statuses->last;
                    tmp_statuses->last = cur_status;
                }
                tmp_statuses->count++;
    }
    return tmp_statuses;
}

static void 
concatStatuses (statuses_t *statuses, statuses_t *tmp_statuses)
{

    if (statuses->count != 0 && tmp_statuses->count != 0) {
        tmp_statuses->last->next = statuses->first;
        statuses->first->prev = tmp_statuses->last;
        statuses->first = tmp_statuses->first;
        statuses->count = statuses->count + tmp_statuses->count;
    } else if (statuses->count == 0 && tmp_statuses->count != 0) {
        *statuses = *tmp_statuses;
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
    statuses_t * statuses       = NULL;
    if ((statuses = initStatuses(statuses)) == NULL) {
        ERROR;
        return EXIT_FAILURE;
    }

    static char *data           = NULL;
    xmlDoc      *xmldoc         = NULL;
    xmlNode     *xmlroot        = NULL;
    statuses_t  *tmp_statuses   = NULL;

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

    if ((tmp_statuses = initStatuses(tmp_statuses)) == NULL) {
        ERROR;
        return EXIT_FAILURE;
    }
    
    xmlInitParser();

    tmp_statuses = getNewStatuses(xmldoc, statuses, tmp_statuses);

    if (tmp_statuses->count != 0) {
        concatStatuses(statuses, tmp_statuses);
        printStatuses(statuses);
    }
    free(tmp_statuses);
    free(data);
    xmlFreeDoc (xmldoc);
    xmlCleanupParser();
    xmlCleanupParser();
    free(statuses);
    return EXIT_SUCCESS;
}
