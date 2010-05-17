#ifndef CURL_H_INCLUDED
#define CURL_H_INCLUDED

char * get_URL (const char *url, config_t *config);
void post_status (char *tweet, config_t *config);

#endif
