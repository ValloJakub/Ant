#ifndef WORLD_H
#define WORLD_H

#include <pthread.h>
#include "ant.h"

#include <stdio.h>
#include <stdlib.h>


#define MAX_ANTS 5  // Počet mravcov
#define STEPS_LIMIT 50 // Počet krokov do konca hry

typedef struct world {
    int size;
    char **grid;
    pthread_mutex_t mutex;
    Ant ants[MAX_ANTS];
} World;

void moveAnt(Ant *ant, World *world);
void initializeWorld(World *world, int size);
void initializeRandomBlackCells(World *world, int num_black_cells);
void initializeRandomAnts(World *world);
void displayWorld(World *world);
void freeWorldMemory(World *world);

void setBlackCell(World *world, int x, int y);



void saveWorldToFile(World *world, const char *filename);
void loadWorldFromFile(World *world, const char *filename);

#endif
