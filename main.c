#include "world.h"
#include "ant.h"
#include "runStop.h"
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <conio.h>
#include <winsock.h>

#define BUFFER_SIZE 256

// Vytvorenie simulácie
void create() {
    srand(time(NULL));

    World world;
    world.stepLimit = 2000;

    pthread_mutex_t ant_mutex;
    pthread_mutex_init(&ant_mutex, NULL);

    int size, num_black_cells, max_ants;
    printf("Enter the size of the world: ");
    scanf("%d", &size);

    if (size < 5) {
        printf("\nWorld is too small! Setting the size as default: 10");
        size = 10;
    } else if (size > 14) {
        printf("\nWorld is too large! Setting the size as default: 10");
        size = 10;
    }

    printf("\nEnter the number of maximum ants: ");
    scanf("%d", &max_ants);

    if (max_ants <= 0 || max_ants > (size * size) / 2 ) {
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
    printf("===============================================\n");

    Ant ants[world.max_ants];

    bool end = false;
    bool stop = false;
    initializeRandomAnts(ants, &world, &end, &stop, &ant_mutex);
    RunStopData run;
    run.mutex = &ant_mutex;
    run.endRunBool = &end;
    run.stopRunBool = &stop;
    run.world = &world;

    pthread_t worldRun;
    pthread_create(&worldRun, NULL, worldThread, &run);

    pthread_t antThreadIds[world.max_ants];
    for (int i = 0; i < world.max_ants; i++) {
        pthread_create(&antThreadIds[i], NULL, antThread, &ants[i]);
    }

    pthread_join(worldRun, NULL);
    for (int i = 0; i < world.max_ants; i++) {
        pthread_join(antThreadIds[i], NULL);
    }

    freeWorldMemory(&world);
    pthread_mutex_destroy(run.mutex);
    for (int i = 0; i < world.max_ants; ++i) {
        pthread_mutex_destroy(ants[i].mutex);
    }
}

// Načítanie sveta
void load(const char *filename) {
    srand(time(NULL));

    World world;
    world.stepLimit = 2000;

    pthread_mutex_t ant_mutex;
    pthread_mutex_init(&ant_mutex, NULL);

    // Načíta svet zo súboru a inicializuje ho ako nový svet
    loadWorldFromFile(&world, filename);
    printf("===============================================\n");

    Ant ants[world.max_ants];
    bool end = false;
    bool stop = false;
    initializeRandomAnts(ants, &world, &end, &stop, &ant_mutex);

    RunStopData run;
    run.mutex = &ant_mutex;
    run.endRunBool = &end;
    run.stopRunBool = &stop;
    run.world = &world;

    pthread_t worldRun;
    pthread_create(&worldRun, NULL, worldThread, &run);

    pthread_t antThreadIds[world.max_ants];
    for (int i = 0; i < world.max_ants; i++) {
        pthread_create(&antThreadIds[i], NULL, antThread, &ants[i]);
    }

    pthread_join(worldRun, NULL);
    for (int i = 0; i < world.max_ants; i++) {
        pthread_join(antThreadIds[i], NULL);
    }

    freeWorldMemory(&world);
    pthread_mutex_destroy(run.mutex);
    for (int i = 0; i < world.max_ants; ++i) {
        pthread_mutex_destroy(ants[i].mutex);
    }
}


void sendCommandToServer(int sockfd, const char* command) {
    if (send(sockfd, command, strlen(command), 0) < 0) {
        perror("Error sending command to the server");
        exit(EXIT_FAILURE);
    }
}

void receiveWorldFromServer(int sockfd, const char* filename) {
    FILE* worldFile = fopen(filename, "wb");
    if (worldFile == NULL) {
        perror("Error opening world file");
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    int n;
    while ((n = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
        if (fwrite(buffer, 1, n, worldFile) < n) {
            perror("Error writing to world file");
            fclose(worldFile);
            closesocket(sockfd);
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        break;
    }

    fclose(worldFile);
    printf("World received from the server and saved to %s.\n", filename);
}

// Vytvorenie spojenia so serverom
int createConnection(int argc, char *argv[]) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("Error initializing Winsock");
        return 1;
    }

    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    if (argc < 4) {
        fprintf(stderr, "usage %s hostname port pattern_filename\n", argv[0]);
        WSACleanup();
        return 1;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        WSACleanup();
        return 2;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(atoi(argv[2]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        perror("Error creating socket");
        WSACleanup();
        return 3;
    }

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error connecting to socket");
        closesocket(sockfd);
        WSACleanup();
        return 4;
    }

    printf("Connected to the Langton server.\n");
    return sockfd;
}

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    int n;
    int sockfd = createConnection(argc, argv);

    char input;
    while (1) {
        printf("Type 'l', if you want to load world from a file.\n");
        printf("Type 'c', if you want to create your own map, that will be saved.\n");
        scanf(" %c", &input);
        if (input == 'l' || input == 'L') {
            printf("\nDo you want to load from local file or server? [l/L] [s/S]\n");
            scanf(" %c", &input);
            if (input == 's' || input == 'S') {
// Načítanie zo servera
                sendCommandToServer(sockfd, "s");
                receiveWorldFromServer(sockfd, "received_file.txt");
                load("received_file.txt");

                printf("\nDo you wish to save the world on server? [y/Y] [n/N]\n");
                while (1) {
                    scanf(" %c", &input);
                    if (input == 'y' || input == 'Y') {
                        printf("\nSaving on server..\n");
                        sockfd = createConnection(argc, argv);
                        sendCommandToServer(sockfd, "u");
// Uloženie na server
                        FILE *worldFile = fopen("world.txt", "rb");
                        if (worldFile == NULL) {
                            perror("Error opening world file");
                            closesocket(sockfd);
                            WSACleanup();
                            return 5;
                        }
                        // Číta dáta zo súboru do buffera
                        while ((n = fread(buffer, 1, sizeof(buffer), worldFile)) > 0) {
                            if (send(sockfd, buffer, n, 0) < 0) {
                                perror("Error sending world file to the server");
                                fclose(worldFile);
                                closesocket(sockfd);
                                WSACleanup();
                                return 6;
                            }
                        }
                        printf("\nEnding simulation..");
                        exit(EXIT_SUCCESS);
                    } else if (input == 'n' || input == 'N') {
                        printf("\nEnding simulation..");
                        exit(EXIT_SUCCESS);
                    }
                }
            }
        } else if (input == 'c' || input == 'C') {
            create();
            printf("\nDo you wish to save the world on server? [y/Y] [n/N]\n");
            while (1) {
                scanf(" %c", &input);
                if (input == 'y' || input == 'Y') {
                    sendCommandToServer(sockfd, "u");
                    printf("\nSaving on server..\n");
// Uloženie na server
                    FILE *worldFile = fopen("world.txt", "rb");
                    if (worldFile == NULL) {
                        perror("Error opening world file");
                        closesocket(sockfd);
                        WSACleanup();
                        return 5;
                    }
                    while ((n = fread(buffer, 1, sizeof(buffer), worldFile)) > 0) {
                        if (send(sockfd, buffer, n, 0) < 0) {
                            perror("Error sending world file to the server");
                            fclose(worldFile);
                            closesocket(sockfd);
                            WSACleanup();
                            return 6;
                        }
                    }
                    printf("\nEnding simulation..");
                    exit(EXIT_SUCCESS);
                } else if (input == 'n' || input == 'N') {
                    printf("\nEnding simulation..");
                    exit(EXIT_SUCCESS);
                }
            }
        }
    }
}
