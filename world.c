
#include <windows.h>

#include "world.h"
#include "ant.h"
#include <stdio.h>
#include <stdlib.h>

void saveWorldToFile(World *world, const char *filename) {
    FILE *file = fopen(filename, "w+");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%d %d\n", world->size, world->max_ants);

    for (int i = 0; i < world->size; i++) {
        for (int j = 0; j < world->size; j++) {
            char outputChar = (world->grid[i][j] == '@') ? '#' : world->grid[i][j]; // Ak sa znak rovná mravcovi, do súboru to zapíše ako čierne políčko
            fprintf(file, "%c", outputChar);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

void loadWorldFromFile(World *world, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%d %d\n", &world->size, &world->max_ants);

    initializeWorld(world, world->size, world->max_ants);

    for (int i = 0; i < world->size; i++) {
        for (int j = 0; j < world->size; j++) {
            fscanf(file, "%c", &world->grid[i][j]);
        }
        fscanf(file, "\n");
    }
    fclose(file);
}

void initializeWorld(World *world, int size, int max_ants) {
    world->size = size;
    world->max_ants = max_ants;

    // Inicializácia mriežky (grid)
    world->grid = (char **)malloc(world->size * sizeof(char *));
    if (world->grid == NULL) {
        perror("Error allocating memory for grid");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; i++) {
        world->grid[i] = (char *)malloc(world->size * sizeof(char));
        if (world->grid[i] == NULL) {
            perror("Error allocating memory for grid row");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < world->size; i++) {
        for (int j = 0; j < world->size; j++) {
            world->grid[i][j] = '.';
        }
    }
}

void freeWorldMemory(World *world) {
    for (int i = 0; i < world->size; i++) {
        free(world->grid[i]);
    }
    free(world->grid);

    printf("Ending simulation..\n");
}

void initializeRandomBlackCells(World *world, int num_black_cells) {
    for (int k = 0; k < num_black_cells; k++) {
        int i = rand() % world->size;
        int j = rand() % world->size;
        world->grid[i][j] = '#';
    }
}

void setBlackCellsManually(World *world, int x, int y) {
    for (int i = 0; i < world->size; ++i) {
        for (int j = 0; j < world->size; ++j) {
            world->grid[x][y] = '#';
        }
    }
}

void initializeRandomAnts(Ant ants[], World *world, pthread_mutex_t *mutex_ant) {
    for (int i = 0; i < world->max_ants; i++) {
        ants[i].mutex = mutex_ant;
        int x, y;
        do {
            x = rand() % world->size;
            y = rand() % world->size;
        } while (world->grid[x][y] == '@');  // Zabezpečuje, že mravec nezačína na mravcovi

        ants[i].lastColor = 'x';
        ants[i].x = x;
        ants[i].y = y;
        ants[i].direction = rand() % 4; // Náhodný smer (0-3)
        ants[i].isDeleted = 0;
        ants[i].world = world;
        world->grid[x][y] = '@';
    }
}

void setConsoleColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetConsoleColor() {
    setConsoleColor(7); // Čierna farba
}

void displayWorld(World *world) {
    for (int i = 0; i < world->size; i++) {
        for (int j = 0; j < world->size; j++) {
            char znak = world->grid[i][j];
            if (znak == '@') {
                // Reprezentácia mravca - červenou farbou
                setConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                printf("@ ");
                resetConsoleColor();
            } else {
                printf("%c ", world->grid[i][j]);
            }
        }
        printf("|\n");
    }
    for (int i = 0; i < world->size; ++i) {
        printf("--");
    }
    printf("|\n");
}

void moveAnt(Ant *ant, World *world) {
    if (ant->isDeleted == 1) {
        ant->x = -1;
        ant->y = -1;
        return;
    }

    char leavingColor;
    // INVERZNÁ LOGIKA POHYBU
    if (ant->lastColor == '.') {
        // Na bielom poli
        ant->direction = (ant->direction + 3) % 4; // Otočenie o 90° vľavo
        leavingColor = '#'; // Zmena bielého pola na čierne
    } else {
        // Na čiernom poli
        ant->direction = (ant->direction + 1) % 4; // Otočenie o 90° vpravo
        leavingColor = '.'; // Zmena čierneho pola na biele
    }

    // Získať novú pozíciu mravca
    int new_x, new_y;
    switch (ant->direction) {
        case 0: // hore
            new_y = (ant->y - 1 + world->size) % world->size;
            new_x = ant->x;
            break;
        case 1: // vpravo
            new_y = ant->y;
            new_x = (ant->x + 1) % world->size;
            break;
        case 2: // dole
            new_y = (ant->y + 1) % world->size;
            new_x = ant->x;
            break;
        case 3: // vlavo
            new_y = ant->y;
            new_x = (ant->x - 1 + world->size) % world->size;
            break;
    }

    pthread_mutex_lock(ant->mutex);

    world->grid[ant->x][ant->y] = leavingColor;
    // Detekcia stretu mravcov - je vymazany
    if (world->grid[new_x][new_y] == '@') {
        ant->isDeleted = 1;
    }
    ant->lastColor = world->grid[new_x][new_y];
    world->grid[new_x][new_y] = '@';

    pthread_mutex_unlock(ant->mutex);

    // Posunutie mravca vpred
    ant->x = new_x;
    ant->y = new_y;
}