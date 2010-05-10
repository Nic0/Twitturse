#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include "utils.h"
#include "config.h"
#include "curl.h"

#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))


char *
get_URL (const char *url, config_t *config)
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
        //curl_easy_setopt (curl, CURLOPT_USERNAME, "nic0sphere");
        //curl_easy_setopt (curl, CURLOPT_PASSWORD, "6akw92c");
        curl_easy_setopt (curl, CURLOPT_USERNAME, config->login);
        curl_easy_setopt (curl, CURLOPT_PASSWORD, config->passwd);
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

void
post_status (char *tweet)
{

  CURL *curl = NULL;
  CURLcode res;
 
  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;
  struct curl_slist *headerlist=NULL;
  static const char buf[] = "Expect:";
 
  curl_global_init(CURL_GLOBAL_ALL);
/* 
  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "source",
               CURLFORM_COPYCONTENTS, "Twitturse",
               CURLFORM_END);*/
  
  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "status",
               CURLFORM_COPYCONTENTS, tweet,
               CURLFORM_END);
 
  /* Fill in the submit field too, even if this is rarely needed 
  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "submit",
               CURLFORM_COPYCONTENTS, "send",
               CURLFORM_END);*/
 
  curl = curl_easy_init();
  /* initalize custom header list (stating that Expect: 100-continue is not
     wanted */ 
  headerlist = curl_slist_append(headerlist, buf);
  if(curl) {
    /* what URL that receives this POST */ 
        curl_easy_setopt (curl, CURLOPT_URL, "http://api.twitter.com/1/statuses/update.xml");
        curl_easy_setopt (curl, CURLOPT_USERNAME, "twitturse");
        curl_easy_setopt (curl, CURLOPT_PASSWORD, "58auau59");
      
      /* only disable 100-continue header if explicitly requested */ 
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    res = curl_easy_perform(curl);
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
 
    /* then cleanup the formpost chain */ 
    curl_formfree(formpost);
    /* free slist */ 
    curl_slist_free_all (headerlist);
  }
}
