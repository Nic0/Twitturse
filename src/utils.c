#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define STRING_BLOCK_SIZE 1024


/* Allocation de mémoire pour une nouvelle chaîne, vide.
 * Correspondance Glib : g_string_new("").
 */
string_t *
string_new (void)
{
  string_t *s;
  if (NULL != (s = malloc (sizeof *s)))
    {
      s->str = NULL;
      s->size = 0;
      s->len = 0;
    }
  return (s);
}

/* Allocation de mémoire pour une nouvelle chaîne,
 * avec pré-allocation d'un bloc de mémoire.
 * La chaîne est vide.
 * Correspondance Glib : g_string_new_len("", STRING_BLOCK_SIZE).
 */
string_t *
string_new_initial_len (void)
{
  string_t *str;
  if (NULL != (str = malloc (sizeof *str)))
    {
      str->str = malloc (STRING_BLOCK_SIZE * sizeof *str->str);
      str->size = STRING_BLOCK_SIZE;
      str->len = 0;
    }
  return (str);
}

/* Libération de la mémoire allouée pour une chaîne
 * Correspondance Glib : g_string_free(str, TRUE).
 */
void
string_free (string_t * str)
{
  if (str)
    {
      if (str->str)
        free (str->str);
      free (str);
    }
}

/* Concaténation d'une chaîne char* à une chaîne string_t */
/* Note : pour concaténer deux chaînes string_t, faire
 * string_ajout(resultat, chaine->str);
 * Correspondance Glib : g_string_append(str, str2).
 */
void
string_ajout (string_t * str, const char *str2)
{
  int l;
  l = strlen (str2);
  if (str->size < (str->len + l + 1))
    {
      str->size =
        (1 + (str->size + l + 1) / STRING_BLOCK_SIZE)
         * STRING_BLOCK_SIZE;
      str->str = realloc (str->str, str->size * sizeof (*str->str));
    }
  memcpy (str->str + str->len, str2, l + 1);
  str->len += l;
}

size_t
str_append (void *ptr, size_t size, size_t nmemb, void *stream)
{
  string_t *str = stream;
  int len = size * nmemb;
  if (str->size < (str->len + len))
    {
      /* Si la chaîne est trop petite, nous l'agrandissons. */
      str->size += (1 + (len / STRING_BLOCK_SIZE))
                   * STRING_BLOCK_SIZE;
      str->str = realloc (str->str, str->size + 1);
      if (NULL == str->str)
        return -1;
    }
/* Ajout des données à la fin de la chaîne et mise à jour de la chaîne.
 */
  memcpy (str->str + str->len, ptr, len);
  str->len += len;
  str->str[str->len] = '\0';
  return (len);
}

char *cat_chaine (char *chaine1, char *chaine2)
{
    char *result;
    int len1, len2;
    len1 = strlen(chaine1);
    len2 = strlen(chaine2);
 
    if(NULL == (result = realloc (chaine1, (len1 + len2 +1) * sizeof *result)))
        return (NULL);
 
    memcpy (result + len1, chaine2, len2 + 1);
    return result;
}
