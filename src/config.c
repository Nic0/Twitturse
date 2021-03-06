#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "init.h"
#include "utils.h"

#define MAXBUFF 512
#define NBR_ELEMENT 3 

#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))


/*  Fonction permettant de récupérer les données lu dans le fichier de configuration
 *  Chaque éléments reconnu est stocké dans la structure Configuration, 
 *  Il lui est rajouté dans cette fonction le nécessaire pour être envoyer directement
 *  dans la socket (une fois sortie de cette fonction, tout est prêt à l'emploi.
 *  Si un # est rencontré en 1er caractère, on considère que c'est un commentaire
 *  et est alors exclu.
 */
int getConfiguration (config_t *config)
{
    FILE *fichier;
    char filebuff[MAXBUFF] = {0};
    int nbrElement = 0;

    /* Default parameter if no refresh found in configuration file
     */
    config->refresh = 60;

    fichier = fopen (config->path_config, "r");
    if (fichier != NULL) {
        while ((fgets (filebuff, MAXBUFF, fichier)) != NULL) {

            if (filebuff[0] == '#')
                continue;


            if ((strncmp (filebuff, "login", 5)) == 0) {
                if ((config->login = (strdup(strchr (filebuff, '"')))) != NULL)
                    if (extractConfig(config->login) == 0) {
                        nbrElement++;
                        continue;
                    }
                ERROR;
                return 1;
            }
            
            if ((strncmp (filebuff, "passwd", 6)) == 0) {
                if ((config->passwd = (strdup(strchr (filebuff, '"')))) != NULL)
                    if (extractConfig(config->passwd) == 0) {
                        nbrElement++;
                        continue;
                    }
                ERROR;
                return 1;
            }

            if ((strncmp (filebuff, "refresh", 7)) == 0) {
            char *tmprefresh;
                if ((tmprefresh = (strdup(strchr(filebuff, '"')))) != NULL)
                    if (extractConfig(tmprefresh) == 0) {
                        config->refresh = atoi(tmprefresh);
                        nbrElement++;
                        continue;
                    }
                ERROR;
                return 1;
            }

        }
        fclose (fichier);
    }
    else {
        ERROR;
        return 1;
    }
    if (nbrElement > NBR_ELEMENT || nbrElement < (NBR_ELEMENT-1)) {
        fprintf (stderr, "The configuration file doesn't seem to contain\
                 enought or to much data\n");
        return 1;
    }
    return 0;
}
/*  Get the home directory path
 */
int
get_config_filedir(config_t *config)
{
    char *buffer;
    if((buffer = getenv ("HOME")) != NULL)
        if (strcat(buffer, "/.twitturserc"))
            if(config->path_config = strdup(buffer))
                return 0;
    ERROR;
    return 1;
}
/* Check if the config file is here, otherwise it will display 
 * an usage for this file.
 */
int
check_configfile(config_t *config)
{
    FILE *file = NULL;
    file = fopen(config->path_config, "r");
    if (file != NULL) {
        fclose(file);
        return 0;
    } else {
        puts("\n\nThe configuration file doesn't seem to be here\n");
        printf("You should have in your %s\n", config->path_config);
        puts("Something similar to:\n");
        puts("login \"mylogin\"\n");
        puts("passwd \"myaccountpass\"\n\n\n");
        return 1;
    }
}

/*  Utilitaire: lorsque l'élément en question est lu dans le fichier de configuration
 *  il apparait sous la forme "element", cette fonction enlève donc les "" pour ne laisser
 *  que l'élément.
 */
int extractConfig (char *element)
{
    int i = 0;
    do {
        element[i] = element[i+1];
        i++;
    }while (element[i+1] != '"' && i <= 1000);

    if (i >= 1000) {
        fprintf(stderr, "An error has occured in the configuration file, a double-quote is missing\n");
        return 1;
    }
    element[i] = '\0';
    return 0;
}

/*  Une fois les informations envoyé, on libère la structure.
 */
void freeStruct (config_t *config)
{
    free(config->login);
    free(config->passwd);
    free(config);
}
