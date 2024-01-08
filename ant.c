#include "world.h"
#include "ant.h"

#include <pthread.h>
#include <unistd.h>

void *antThread(void *arg) {
    Ant *ant = (Ant *)arg;
    World *world = ant->world;

        pthread_mutex_lock(&world->mutex);

        moveAnt(ant, world);

        pthread_mutex_unlock(&world->mutex);

    return NULL;
}