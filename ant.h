#ifndef ANT_H
#define ANT_H

#include <stdbool.h>
#include "world.h"

typedef struct ant {
    bool *end;
    bool *stop;
    int x;
    int y;
    int direction;
    int isDeleted;
    char lastColor;
    struct World *world;
    pthread_mutex_t *mutex;
} Ant;

void *antThread(void *arg);

#endif
