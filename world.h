#ifndef WORLD_H
#define WORLD_H

#include <pthread.h>
#include "ant.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct World {
    int stepLimit;
    int max_ants;
    int size;
    char **grid;
} World;

void moveAnt(Ant *ant, World *world);

void initializeRandomBlackCells(World *world, int num_black_cells);
void initializeWorld(World *world, int size, int max_ants);
void initializeRandomAnts(Ant ants[], World *world, bool *end, bool *stop, pthread_mutex_t *mutex_ant);
void displayWorld(World *world);
void setBlackCellsManually(World *world, int x, int y);

void freeWorldMemory(World *world/*, Ant *ant*/);

void saveWorldToFile(World *world, const char *filename);
void loadWorldFromFile(World *world, const char *filename);

#endif
