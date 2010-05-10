#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

typedef struct
{
    char *login;
    char *passwd;
} config_t;

int
getConfiguration (config_t *config);

int 
extractConfig(char *element);

void
freeStruct (config_t *config);

#endif
