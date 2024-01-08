#include "world.h"
#include "ant.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <windows.h>

void saveWorldToFile(World *world, const char *filename) {
    FILE *file = fopen(filename, "w+");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%d \n", world->size);

    for (int i = 0; i < world->size; i++) {
        for (int j = 0; j < world->size; j++) {
            fprintf(file, "%c ", world->grid[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

void setBlackCell(World *world, int x, int y) {
    if (x >= 0 && x < world->size && y >= 0 && y < world->size) {
        world->grid[y][x] = '#';
    }
}

void loadWorldFromFile(World *world, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%d ", &world->size);

    initializeWorld(world, world->size);

    for (int i = 0; i < world->size; i++) {
        for (int j = 0; j < world->size; j++) {
            fscanf(file, "%c ", &world->grid[i][j]);
          /*  if (world->grid[i][j] == '#') {
                setBlackCell(world, j, i);
            }*/
        }
    }
    fclose(file);
}

void initializeWorld(World *world, int size) {
    // Musí byť naopak
    world->size = size;

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
            world->grid[i][j] = ' ';
        }
    }

    // Inicializácia mutexu
    pthread_mutex_init(&world->mutex, NULL);
}

void freeWorldMemory(World *world) {
    for (int i = 0; i < world->size; i++) {
        free(world->grid[i]);
    }
    free(world->grid);
}

void initializeRandomBlackCells(World *world, int num_black_cells) {
    srand(time(NULL));
    for (int k = 0; k < num_black_cells; k++) {
        int i = rand() % world->size;
        int j = rand() % world->size;
        world->grid[i][j] = '#';
    }
}

void initializeRandomAnts(World *world) {
    for (int i = 0; i < MAX_ANTS; i++) {
        int x, y;
        do {
            x = rand() % world->size;
            y = rand() % world->size;
        } while (world->grid[y][x] == '@');  // Zabezpečuje, že mravec nezačína na mravcovi

        world->ants[i].x = x;
        world->ants[i].y = y;
        world->ants[i].direction = rand() % 4; // Náhodný smer (0-3)
        world->ants[i].isDeleted = 0;
        world->ants[i].world = world;
    }
}

// Function to set console text color
void setConsoleColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// Function to reset console text color to default
void resetConsoleColor() {
    setConsoleColor(7); // Assuming 7 is the default color (white on black)
}

void displayWorld(World *world) {
    for (int i = 0; i < world->size; i++) {
        for (int j = 0; j < world->size; j++) {
            int antIndex = -1;
            for (int k = 0; k < MAX_ANTS; k++) {
                if (world->ants[k].x == j && world->ants[k].y == i) {
                    antIndex = k;
                    break;
                }
            }

            if (antIndex != -1 && !world->ants[antIndex].isDeleted) {
                // Reprezentácia mravca - červenou farbou
                setConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                printf("@ ", 'A');
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

int antsDead = 0;

void moveAnt(Ant *ant, World *world) {
    if (ant->isDeleted == 1) {
        ant->x = -1;
        ant->y = -1;
        return;
    }

    // INVERZNÁ LOGIKA POHYBU
    if (world->grid[ant->y][ant->x] == ' ') {
        // Na bielom poli
        ant->direction = (ant->direction + 3) % 4; // Otočenie o 90° vľavo
        world->grid[ant->y][ant->x] = '#'; // Zmena bielého pola na čierne
    } else {
        // Na čiernom poli
        ant->direction = (ant->direction + 1) % 4; // Otočenie o 90° vpravo
        world->grid[ant->y][ant->x] = ' '; // Zmena čierneho pola na biele
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

    // Detekcia stretu mravcov
    for (int i = 0; i < MAX_ANTS; i++) {
        if (i != ant - world->ants && world->ants[i].x == new_x && world->ants[i].y == new_y) {
            // Stret mravcov, vymaž oboch mravcov
            ant->isDeleted = 1;
            world->ants[i].isDeleted = 1;
            antsDead += 2;

            // Označenie mŕtvych mravcov
            for (int j = 0; j < MAX_ANTS; j++) {
                if (world->ants[j].isDeleted) {
                    world->ants[j].x = -1;
                    world->ants[j].y = -1;
                }
            }

            // Kontrola či sú všetky mravce mŕtve
            if (antsDead == MAX_ANTS) {
                printf("All ants are dead! Ending simulation..\n");
                exit(EXIT_SUCCESS);
            }
        }
    }
    // Posunutie mravca vpred
    ant->x = new_x;
    ant->y = new_y;
}