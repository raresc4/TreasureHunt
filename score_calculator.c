#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    int x;
    int y;
}Coordonate;

typedef struct {
    int id;
    char username[50];
    Coordonate coordonate;
    char clue[100];
    int value;
}Treasure;

typedef struct {
    char username[50];
    int score;
}UserScore;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <hunt_name>\n", argv[0]);
        return 1;
    }

    char hunt_name[50];
    strncpy(hunt_name, argv[1], sizeof(hunt_name) - 1);
    hunt_name[sizeof(hunt_name) - 1] = '\0';

    char filepath[150];
    sprintf(filepath, "%s/treasures", hunt_name);

    int treasureFd = open(filepath, O_RDONLY);
    if (treasureFd < 0) {
        fprintf(stderr, "Error opening treasures file for hunt: %s\n", hunt_name);
        return 1;
    }

    UserScore users[100]; 
    int user_count = 0;

    Treasure tr;
    ssize_t bytesRead;

    while ((bytesRead = read(treasureFd, &tr, sizeof(Treasure))) == sizeof(Treasure)) {
        int user_found = 0;
        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].username, tr.username) == 0) {
                users[i].score += tr.value;
                user_found = 1;
                break;
            }
        }

        if (!user_found && user_count < 100) {
            strncpy(users[user_count].username, tr.username, sizeof(users[user_count].username) - 1);
            users[user_count].username[sizeof(users[user_count].username) - 1] = '\0';
            users[user_count].score = tr.value;
            user_count++;
        }
    }

    close(treasureFd);

    if (user_count == 0) {
        printf("No users found in hunt: %s\n", hunt_name);
        return 0;
    }

    printf("SCORES FOR HUNT: %s\n", hunt_name);
    for (int i = 0; i < user_count; i++) {
        printf("User: %s, Score: %d\n", users[i].username, users[i].score);
    }
    printf("\n");

    return 0;
}
