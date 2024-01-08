#include "world.h"
#include "ant.h"
#include <pthread.h>

void *antThread(void *arg) {
    Ant *ant = (Ant *)arg;
    World *world = ant[0].world;

    pthread_mutex_lock(ant->mutex);
    while (world->stepLimit > 0) {
        world->stepLimit--;

        pthread_mutex_unlock(ant->mutex);

        // Ak je stop true, tak vlákna čakajú
        bool stop = *(ant->stop);
        while (stop) {
            stop = *(ant->stop);
        }

        bool end = *(ant->end);
        if (ant->isDeleted == 1 || end) {
            pthread_mutex_unlock(ant->mutex);
            return NULL;
        }

        moveAnt(ant, ant->world);

        pthread_mutex_lock(ant->mutex);

        displayWorld(world);
    }
    pthread_mutex_unlock(ant->mutex);

    return NULL;
}
