#ifndef ANT_H
#define ANT_H

#include "world.h"

typedef struct ant {
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
