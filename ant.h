#ifndef ANT_H
#define ANT_H

#include "world.h"

typedef struct ant {
    int x;
    int y;
    int direction;
    int isDeleted;
    struct World *world;
} Ant;

void *antThread(void *arg);

#endif
