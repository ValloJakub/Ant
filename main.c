#include "world.h"
#include "ant.h"
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

int main() {
    World world;
/*
    int size, num_black_cells;

    printf("Enter the size of the world: ");
    scanf("%d", &size);

    if (size < 5 ) {
        printf("World is too small! Setting the size as default: 10\n");
        size = 10;
    }

    printf("Enter the number of initial black cells: ");
    scanf("%d", &num_black_cells);

    if (num_black_cells < 0 || num_black_cells > size * size) {
        printf("Invalid input. Setting the number of black cells as 0\n");
        num_black_cells = 0;
        usleep(2000000);
    }*/

    // Načíta svet zo súboru a inicializuje ho ako nový svet
    loadWorldFromFile(&world, "world.txt");

    //initializeWorld(&world, size);
    //initializeRandomBlackCells(&world, num_black_cells);
    initializeRandomAnts(&world);

    pthread_t antThreadIds[MAX_ANTS];
    for (int i = 0; i < MAX_ANTS; i++) {
        pthread_create(&antThreadIds[i], NULL, antThread, (void *)&world.ants[i]);
    }

   // Vykreslí svet až po tom, ako sa pohli všetky mravce
    int steps = 1;
    while (steps < STEPS_LIMIT) {
        for (int i = 0; i < MAX_ANTS; i++) {
            moveAnt(&world.ants[i], &world);
        }
        displayWorld(&world);
        steps++;
    }

    // Po skončení simulácie uloží svet do lokálneho súboru
   // saveWorldToFile(&world, "world.txt");


    freeWorldMemory(&world);
    pthread_mutex_destroy(&world.mutex);

    return 0;
}