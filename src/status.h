#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED

#include "init.h"

void *
getNewStatuses (void *data);

void
printStatuses (statuses_t *statuses);

void
clear_statuses (data_t *data);
#endif
