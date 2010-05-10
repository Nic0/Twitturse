/***********************************************************
*
*       Twitturse   v 0.0.8-2
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
#include <pthread.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "config.h"
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

typedef struct
{
    statuses_t  *statuses;
    config_t    *config;
} data_t;


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

config_t *
initConfig (config_t *config)
{
    if ((config = malloc(sizeof(config_t))) == NULL)
        return NULL;
    config->login   = NULL;
    config->passwd  = NULL;
    return config;
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

data_t *
initData (data_t *data)
{
    if ((data = malloc (sizeof(data_t))) == NULL)
        return NULL;
    if ((data->statuses = initStatuses(data->statuses)) == NULL)
        return NULL;
    if ((data->config = initConfig(data->config)) == NULL)
        return NULL;
    return data;
}

void *
getNewStatuses (data_t *data)
{
    while (1) {
    char    *urldoc = NULL;
    xmlDoc  *xmldoc = NULL;
    char    *url    = NULL;
    url = "http://api.twitter.com/statuses/home_timeline.xml";
    urldoc = get_URL (url, data->config);

    if ((xmldoc = xmlParseMemory (urldoc, strlen(urldoc))) == NULL)
        ERROR;

    free(urldoc);

    xmlXPathContextPtr xpathCtx = NULL;
    
    xmlXPathObjectPtr id    = NULL;
    xmlXPathObjectPtr pseudo = NULL;
    xmlXPathObjectPtr text = NULL;

    statuses_t * tmp_statuses;
    tmp_statuses = initStatuses(tmp_statuses);

    xpathCtx = xmlXPathNewContext(xmldoc);
    if (xmldoc == NULL)
        ERROR;
    
    xmlChar *idpath = NULL;
    idpath = "/statuses/status/id/text()";
    xmlChar *textpath = NULL;
    textpath = "/statuses/status/text/text()";
    xmlChar *pseudopath = NULL;
    pseudopath = "/statuses/status/user/screen_name/text()";

    id = xmlXPathEvalExpression(idpath, xpathCtx);
    if (id == NULL)
        ERROR;
    
    text = xmlXPathEvalExpression(textpath, xpathCtx);
    if (text == NULL)
        ERROR;

    pseudo = xmlXPathEvalExpression(pseudopath, xpathCtx);
    if (pseudo == NULL)
        ERROR;

    int size = 0;
    int i;

    size = text->nodesetval ? text->nodesetval->nodeNr : 0;

    for (i = 0; i < size; ++i) {
        //printf ("<%s>\t", pseudo->nodesetval->nodeTab[i]->content);
        //printf ("%s\n", text->nodesetval->nodeTab[i]->content);
        if(data->statuses->count != 0)
            if((xmlStrcmp(id->nodesetval->nodeTab[i]->content, 
                data->statuses->first->id)) == 0)
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

    if(data->statuses->count == 0 && tmp_statuses->count != 0) {
        *data->statuses = *tmp_statuses;
    } else if (data->statuses->count !=0 && tmp_statuses->count != 0) {
        tmp_statuses->last->next = data->statuses->first;
        data->statuses->first->prev = tmp_statuses->last;
        data->statuses->first = tmp_statuses->first;
        data->statuses->count = data->statuses->count + tmp_statuses->count;
    }
    
    xmlXPathFreeObject(pseudo);
    xmlXPathFreeObject(text);
    xmlXPathFreeObject(id);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(xmldoc);
    printStatuses(data->statuses);
    sleep(5);
    }
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
    data_t *data = NULL;
    if ((data = initData(data)) == NULL) {
        ERROR;
        return EXIT_FAILURE;
    }
    if ((getConfiguration(data->config)) != 0)
        return EXIT_FAILURE;

        xmlInitParser();

        pthread_t pidStatuses;
        if (pthread_create(&pidStatuses, NULL, getNewStatuses, data) != 0) {
            ERROR;
            return EXIT_FAILURE;
        }
        
/*
   puts("tweet>");
   char tweet[141] = {0};
   gets(tweet);
   printf("your tweet:%s", tweet);
    post_status (tweet);*/

    pthread_join (pidStatuses, NULL);

    freeStatuses(data->statuses);
    
    return EXIT_SUCCESS;
}
