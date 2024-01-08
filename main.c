#include "world.h"
#include "ant.h"
#include <pthread.h>
#include <unistd.h>

//#include <sys/types.h>
//#include <winsock.h>

void create() {
    srand(time(NULL));

    World world;
    world.stepLimit = 1500;

    pthread_mutex_t ant_mutex;
    pthread_mutex_init(&ant_mutex, NULL);

    int size, num_black_cells, max_ants;
    printf("Enter the size of the world: ");
    scanf("%d", &size);

    if (size < 5) {
        printf("\nWorld is too small! Setting the size as default: 10");
        size = 10;
    }

    printf("\nEnter the number of maximum ants: ");
    scanf("%d", &max_ants);

    if (max_ants <= 0) {
        printf("\nInvalid input. Setting the amount of ants as default: 3");
        max_ants = 3;
        usleep(2000000);
    }

    printf("\nEnter the number of initial black cells: ");
    scanf("%d", &num_black_cells);

    if (num_black_cells < 0 || num_black_cells > size * size) {
        printf("\nInvalid input. Setting the number of black cells as 0 ");
        num_black_cells = 0;
        usleep(2000000);
    }

    // Inicializuj svet
    initializeWorld(&world, size, max_ants);

    if (num_black_cells > 0) {
        char input;
        printf("\nType [m/M] for manual setting: ");
        printf("\nType [r/R] for random setting: ");
        scanf(" %c", &input);

        while (1) {
            if (input == 'm' || input == 'M') {
                for (int i = 0; i < num_black_cells; ++i) {
                    printf("\nEnter coordinates to set black cells as [x y]: \n");
                    int x, y;
                    scanf("%d %d", &x, &y);
                    setBlackCellsManually(&world, x, y);
                    displayWorld(&world);
                }
                break;
            } else if (input == 'r' || input == 'R') {
                initializeRandomBlackCells(&world, num_black_cells);
                break;
            } else {
                printf("\nInvalid input. Try again");
            }
        }
    }
    printf("===================================\n");

    Ant ants[world.max_ants];
    initializeRandomAnts(ants, &world, &ant_mutex);
    displayWorld(&world);

    pthread_t antThreadIds[world.max_ants];
    for (int i = 0; i < world.max_ants; i++) {
        pthread_create(&antThreadIds[i], NULL, antThread, &ants[i]);
    }

    for (int i = 0; i < world.max_ants; i++) {
        pthread_join(antThreadIds[i], NULL);
    }

    // Po skončení simulácie uloží svet do lokálneho súboru
    saveWorldToFile(&world, "world.txt");

    freeWorldMemory(&world);
}

void load() {
    srand(time(NULL));

    World world;
    world.stepLimit = 1500;

    pthread_mutex_t ant_mutex;
    pthread_mutex_init(&ant_mutex, NULL);

    // Načíta svet zo súboru a inicializuje ho ako nový svet
    loadWorldFromFile(&world, "world.txt");

    Ant ants[world.max_ants];
    initializeRandomAnts(ants, &world, &ant_mutex);

    pthread_t antThreadIds[world.max_ants];
    for (int i = 0; i < world.max_ants; i++) {
        pthread_create(&antThreadIds[i], NULL, antThread, &ants[i]);
    }

    for (int i = 0; i < world.max_ants; i++) {
        pthread_join(antThreadIds[i], NULL);
    }

    freeWorldMemory(&world);
}

int main() {
    printf("Type 'l', if you want to load world from a file.\n");
    printf("Type 'c', if you want to create your own map, that will be saved.\n");

    char input;
    while (1) {
        scanf(" %c", &input);
        if (input == 'l' || input == 'L') {
            load();
            break;
        } else if (input == 'c' || input == 'C') {
            create();
            printf("\nDo you want to save world on server? [y/Y] [n/N]");
            char input;
            while (1) {
                scanf(" %c", &input);
                if (input == 'y' || input == 'Y') {
                    printf("Saving on server..\n");
                    break;
                } else if (input == 'n' || input == 'N') {
                    break;
                }
            }
            return 0;
        }
    }
}
