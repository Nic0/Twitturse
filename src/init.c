#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include "init.h"
#include "config.h"

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
    config->path_config     = NULL;
    config->login           = NULL;
    config->passwd          = NULL;
    return config;
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

void freeStatuses (statuses_t *statuses)
{
    free(statuses->last);
    free(statuses->first);
    free(statuses);
}
