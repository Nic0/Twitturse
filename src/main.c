/***********************************************************
*
*       Twitturse   v 0.0.6
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
    xmlChar         *id;
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

status_t* 
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

void freeStatuses (statuses_t *statuses)
{
    free(statuses->last);
    free(statuses->first);
    free(statuses);
}

statuses_t*
initStatuses (statuses_t *statuses)
{
    if ((statuses = malloc (sizeof(statuses_t))) == NULL)
        return NULL;
    statuses->count = 0;
    statuses->first = NULL;
    statuses->last  = NULL;
    return statuses;
}

statuses_t* 
getNewStatuses (xmlDoc *xmldoc, statuses_t *statuses)
{
    xmlXPathContextPtr xpathCtx = NULL;
    
    xmlXPathObjectPtr id    = NULL;
    xmlXPathObjectPtr pseudo = NULL;
    xmlXPathObjectPtr text = NULL;

    statuses_t * tmp_statuses;
    tmp_statuses = initStatuses(tmp_statuses);

    xpathCtx = xmlXPathNewContext(xmldoc);
    if (xmldoc == NULL) {
        ERROR;
        return NULL;
    }
    xmlChar *idpath = NULL;
    idpath = "/statuses/status/id/text()";
    xmlChar *textpath = NULL;
    textpath = "/statuses/status/text/text()";
    xmlChar *pseudopath = NULL;
    pseudopath = "/statuses/status/user/screen_name/text()";

    id = xmlXPathEvalExpression(idpath, xpathCtx);
    if (id == NULL) {
        ERROR;
        return NULL;
    }
    
    text = xmlXPathEvalExpression(textpath, xpathCtx);
    if (text == NULL) {
        ERROR;
        return NULL;
    }

    pseudo = xmlXPathEvalExpression(pseudopath, xpathCtx);
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
            if((xmlStrcmp(id->nodesetval->nodeTab[i]->content, 
                statuses->first->id)) == 0)
                break;

                status_t *cur_status = NULL;
                cur_status = initStatus(cur_status);
                
                cur_status->id = strdup(id->nodesetval->nodeTab[i]->content);
                cur_status->text = strdup(text->nodesetval->nodeTab[i]->content);
                cur_status->pseudo = strdup(pseudo->nodesetval->nodeTab[i]->content);

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

    if(statuses->count == 0 && tmp_statuses->count != 0) {
        *statuses = *tmp_statuses;
    } else if (statuses->count !=0 && tmp_statuses->count != 0) {
        tmp_statuses->last->next = statuses->first;
        statuses->first->prev = tmp_statuses->last;
        statuses->first = tmp_statuses->first;
        statuses->count = statuses->count + tmp_statuses->count;
    }
    
    xmlXPathFreeObject(pseudo);
    xmlXPathFreeObject(text);
    xmlXPathFreeObject(id);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(xmldoc);
    return statuses;
}

void
printStatuses (statuses_t *statuses)
{
    status_t *status = NULL;
    status = statuses->last;
    while (1) {
        printf("<id:%s>\t<%s>\t%s\n", status->id, status->pseudo, status->text);
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
        xmlInitParser();

    while(1) {
        char    *data           = NULL;
        xmlDoc  *xmldoc         = NULL;

        data = get_URL ("http://api.twitter.com/statuses/home_timeline.xml");

        if ((xmldoc = xmlParseMemory (data, strlen(data))) == NULL) {
            ERROR;
            return EXIT_FAILURE;
        }

        statuses = getNewStatuses(xmldoc, statuses);
        printStatuses(statuses);
    
        free(data);
        //xmlCleanupParser();
        sleep(5);
    }
    freeStatuses(statuses);
    return EXIT_SUCCESS;
}
