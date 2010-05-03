#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <curl/curl.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "utils.h"

#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))
/*
typedef struct status_t
{
    char    *pseudo;
    char    *text;
    struct   status_t *next;
    struct   status_t *prev;
} status_t;

status_t * 
initStatus (status_t *status)
{
    status->pseudo = NULL;
    status->text = NULL;
    status->next = (maloc sizeof(status_t));
    status->prev = (maloc sizeof(status_t));
}*/

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


void 
proceed (xmlNodePtr node)
{
    if ((node->type == XML_TEXT_NODE) || (node->type == XML_CDATA_SECTION_NODE)) {
        xmlChar *path = xmlGetNodePath (node);
        //const xmlChar *name;
        //name = "text";

        //if (xmlGetProp(node, name)) {
            //printf ("name: %s\n", node->name);
                //&& strcmp (node->parent->name, "status") != NULL)
                //printf ("%s -> '%s\n", path, node->content ? (char *) node->content : "(null)");
            //if ((!strcmp (node->name, "text"))  &&
                //(!strcmp (node->next->name, "text"))) 
                printf ("%s -> '%s' (%s)\n", path, node->content, node->name);
            
            xmlFree (path);
    }

    
}


void
traverse (xmlNodePtr node)
{   
    xmlNodePtr n = NULL;
    for (n = node; n; n = n->next) {
        if ((n->type == XML_ELEMENT_NODE) && (n->children)) {
            //proceed(n);
            traverse (n->children);
        }
        proceed(n);

        //printf ("attr:%s\n", n->name);
        //printf ("cont:%s\n", n->content);
        /*xmlChar *name_text = "text";

        if (n->name == name_text)
            printf ("tweet:%s\n", n->content);*/
    }
}

static void
print_element_names(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;


    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    
        if (cur_node->parent != NULL) { 
        
            
            if (cur_node->type == XML_TEXT_NODE 
                 && strcmp(cur_node->parent->name, "screen_name") == 0) {
                printf ("<%s>\t", cur_node->content);
            }
            
            if (cur_node->type == XML_TEXT_NODE 
                 && strcmp(cur_node->parent->name, "text") == 0) {
                printf ("%s\n", cur_node->content);
            }

        }

        print_element_names(cur_node->children);
    }
}



int main (void)
{
    char *data;
    data = get_URL ("https://api.twitter.com/statuses/home_timeline.xml");
    //data = get_URL ("http://search.twitter.com/search.atom?q=%23Archlinux");

    xmlDocPtr xmldoc;
    xmlNode *xmlroot = NULL;

    if ((xmldoc = xmlParseMemory (data, strlen(data))) == NULL) {
        ERROR;
        return EXIT_FAILURE;
    }

    if ((xmlroot = xmlDocGetRootElement (xmldoc)) == NULL) {
        fprintf (stderr, "Le document est vide\n");
        xmlFreeDoc (xmldoc);
        return EXIT_FAILURE;
    }

        //printf ("%s", data);
    
    //traverse (xmlroot);
    //printf("root:%s\n", xmlroot->name);
    print_element_names(xmlroot);
    
    xmlFreeDoc (xmldoc);
    xmlCleanupParser();    
    return EXIT_SUCCESS;
}
