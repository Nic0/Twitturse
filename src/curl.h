#ifndef CURL_H_INCLUDED
#define CURL_H_INCLUDED

#include "status.h"

char * 
get_URL (const char *url, config_t *config);

void 
post_status (char *tweet, config_t *config);

void
post_retweet (char *tweet, config_t *config, status_t *retweet_status);
#endif
