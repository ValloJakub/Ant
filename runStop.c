
#include <unistd.h>
#include <conio.h>
#include "runStop.h"

// Funkcia pre vlákno sveta
void *worldThread(void *arg) {
    RunStopData *run = (RunStopData *)arg;

    while (1) {
        usleep(200000);
        // Čítanie klávesnice asynchrónne
        char c;
        if(kbhit()) {
            *run->stopRunBool = true;
            c = getch();
            usleep(500000);
            printf("Simulation has been stopped.\n");
            printf("Do you wish to resume the simulation? [y/Y] [n/N]\n");
            char input;
            while (1) {
                scanf(" %c", &input);
                if (input == 'y' || input == 'Y') {
                    *run->stopRunBool = false;
                    break;
                } else if (input == 'n' || input == 'N') {
                    printf("Do you wish to save the world to local file? [y/Y] [n/N]\n");
                    while (1) {
                        scanf(" %c", &input);
                        if (input == 'y' || input == 'Y') {
                            printf("Saving to local file..\n");
                            saveWorldToFile(run->world, "world.txt");
                            *run->endRunBool = true;    // ukončí
                            *run->stopRunBool = false;  // zastaví - zacyklia sa
                            return NULL;
                        } else if (input == 'n' || input == 'N') {
                            *run->endRunBool =  true;
                            *run->stopRunBool = false;
                            return NULL;
                        }
                    }
                }
            }
        }
    }
}
