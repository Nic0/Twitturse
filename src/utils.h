#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

typedef struct
{
  char *str;                    /* Chaîne */
  int size;                     /* Taille de l'espace alloué */
  int len;                      /* Longueur de la chaîne */
} string_t;
/*----------------------------------------------------*/

string_t * string_new (void);
void string_free (string_t * str);
void string_ajout (string_t * str, const char *str2);
size_t str_append (void *ptr, size_t size, size_t nmemb, void *stream);

#endif
