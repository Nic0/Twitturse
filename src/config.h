#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

typedef struct
{
    char *path_config;
    char *login;
    char *passwd;
    int refresh;
} config_t;

int
getConfiguration (config_t *config);

int
get_config_filedir(config_t *config);

int
check_configfile(config_t *config);

int 
extractConfig(char *element);

void
freeStruct (config_t *config);

#endif
