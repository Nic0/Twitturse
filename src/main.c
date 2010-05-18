/***********************************************************
*
*       Twitturse   v 0.0.17
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
//#include <libintl.h>
#include <locale.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>


#include "init.h"
#include "status.h"
#include "config.h"
#include "curl.h"
#include "ncurse.h"

#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))

pthread_mutex_t mutex;

int 
main (void)
{
    setlocale (LC_ALL, "");
    data_t *data = NULL;

    if ((data = initData(data)) == NULL) {
        ERROR;
        return EXIT_FAILURE;
    }

    if ((get_config_filedir(data->config)) != 0)
        return EXIT_FAILURE;

    if ((check_configfile(data->config)) != 0)
        return EXIT_FAILURE;

    if ((getConfiguration(data->config)) != 0)
        return EXIT_FAILURE;

    xmlInitParser();

    pthread_t pidStatuses;
    if (pthread_create(&pidStatuses, NULL, getNewStatuses, data) != 0) {
        ERROR;
        return EXIT_FAILURE;
    }

    pthread_t pidCurse;
    if (pthread_create(&pidCurse, NULL, ncurseApplication, data) != 0) {
        ERROR;
        return EXIT_FAILURE;
    }
    
    //pthread_join (pidStatuses, NULL);
    pthread_join (pidCurse, NULL);

    freeStatuses(data->statuses);
    
    return EXIT_SUCCESS;
}
