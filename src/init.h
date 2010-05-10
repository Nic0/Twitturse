#ifndef INIT_H_INCLUDED
#define INIT_H_INCLUDED

#include <libxml/tree.h>
#include <libxml/parser.h>
#include "config.h"

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

typedef struct data_t
{
    statuses_t  *statuses;
    config_t    *config;
} data_t;

status_t* 
initStatus (status_t *status);

config_t *
initConfig (config_t *config);

statuses_t*
initStatuses (statuses_t *statuses);

data_t *
initData (data_t *data);

void 
freeStatuses (statuses_t *statuses);

#endif
