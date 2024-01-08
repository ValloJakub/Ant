
#include <pthread.h>
#include "world.h"

typedef struct runStopData {
    pthread_mutex_t *mutex;
    World *world;
    bool *endRunBool;
    bool *stopRunBool;
} RunStopData;

void *worldThread(void *arg);