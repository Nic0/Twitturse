#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <curl/curl.h>

#include "utils.h"
#include "curl.h"

#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))


char *
get_URL (const char *url)
{
    CURL *curl = NULL;
    CURLcode result;
    string_t *str;
    char *res;

    if (NULL == (str = string_new ())) {
        ERROR;
        return NULL;
    }

    curl = curl_easy_init ();

    if (curl != NULL) {
        curl_easy_setopt (curl, CURLOPT_URL, url);
        curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt (curl, CURLOPT_USERNAME, "nic0sphere");
        curl_easy_setopt (curl, CURLOPT_PASSWORD, "6akw92c");
        curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, str_append);
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, str);

        result = curl_easy_perform (curl);
        curl_easy_cleanup (curl);

        if (CURLE_OK == result && (str->str) != NULL) {
            res = str->str;
            free(str);
            return res;
        } else {
            ERROR;
            return NULL;
        }
    } else {
        free (str);
        ERROR;
        return NULL;
    }
}
