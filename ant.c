#include "world.h"
#include "ant.h"

#include <pthread.h>
#include <unistd.h>
#include <conio.h>

void *antThread(void *arg) {
    Ant *ant = (Ant *)arg;
    World *world = ant[0].world;

    pthread_mutex_lock(ant->mutex);
    while (world->stepLimit > 0) {
        world->stepLimit--;

        pthread_mutex_unlock(ant->mutex);
        if (ant->isDeleted == 1) {
            return NULL;
        }
        usleep(100000);

        moveAnt(ant, ant->world);

        pthread_mutex_lock(ant->mutex);

        displayWorld(world);

        // Čítanie klávesnice asynchrónne
        char c;
        if(kbhit()) {
            c = getch();
            printf("Simulation has been stopped.\n");

            printf("Do you wish to resume the simulation? [y/Y] [n/N]\n");
            char input;
            while (1) {
                scanf(" %c", &input);
                if (input == 'y' || input == 'Y') {
                    break;
                } else if (input == 'n' || input == 'N') {
                    printf("Saving..\n");
                    saveWorldToFile(world, "world.txt");
                    exit(EXIT_SUCCESS);
                }
            }
        }
    }
    pthread_mutex_unlock(ant->mutex);

    return NULL;
}
