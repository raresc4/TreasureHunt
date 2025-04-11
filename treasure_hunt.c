#include <stdio.h>   
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
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

Treasure createTreasure() {
    printf("Introdu id-ul:\n");
    Treasure treasure;
    scanf("%d", &treasure.id);
    printf("Introdu username-ul:\n");
    scanf("%s", treasure.username);
    printf("Introdu coordonatele:\n");
    printf("x coordonata:\n");
    scanf("%d", &treasure.coordonate.x);
    printf("y coordonata:\n");
    scanf("%d", &treasure.coordonate.y);
    printf("Introdu indiciul:\n");
    scanf("%s", treasure.clue);
    printf("Introdu valoarea:\n");
    scanf("%d", &treasure.value);
    return treasure;
}

void showTreasure(Treasure *tr) {
    printf("id: %d\nusername: %s\nx coordonate: %d\ny coordonate: %d\nclue: %s\nvalue: %d\n\n", tr->id, tr->username, tr->coordonate.x,tr->coordonate.y,tr->clue,tr->value);
}

void add(char *hunt) {
    DIR *dir = opendir(hunt);
    char newDirPath[100];
    char originalLoggingFile[150];
    char targetLoggingFile[150];
    sprintf(originalLoggingFile, "logging-file-%s.txt", hunt);
    int new = 0;
    if(!dir) {
        new = 1;
        sprintf(newDirPath,"./%s",hunt);
        sprintf(targetLoggingFile, "%s/logging-file.txt", newDirPath);
        if(mkdir(newDirPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
            perror("eroarea la crearea noului hunt");
            return;
        }
    }
    sprintf(targetLoggingFile, "%s/logging-file.txt", hunt);
    int targetLoggingFd = open(targetLoggingFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(targetLoggingFd < 0) {
        perror("eroare la deschiderea fisierului de logare");
        return;
    }
    
    if(new) {
        dir = opendir(newDirPath);
        if(!dir) {
            perror("eroare la deschiderea directorului nou creat");
            return;
        }
        if(symlink(targetLoggingFile, originalLoggingFile) < 0) {
            perror("eroare la crearea legaturii simbolice");
            close(targetLoggingFd);
            return;
        }
    }
    char filepath[150];
    if(new) {
        sprintf(filepath, "%s/%s", newDirPath, "treasures");
    } else {
        sprintf(filepath, "%s/treasures", hunt);    
    }
    int treasureFd = open(filepath, O_RDWR | O_CREAT | O_APPEND, 0644);
    if(treasureFd < 0) {
        perror("eroare la deschiderea sau crearea fisierului treasures");
        close(targetLoggingFd);
        return;
    }
    Treasure tr = createTreasure();
    if(write(treasureFd, &tr, sizeof(Treasure)) != sizeof(Treasure)) {
        perror("eroare la scrierea in fisierul treasures");
        close(treasureFd);
        close(targetLoggingFd);
        return;
    }
    char logMessage[100];
    int len = sprintf(logMessage, "Adaugata comoara cu id-ul %d\n", tr.id);
    if(write(targetLoggingFd, logMessage, len) != len) {
        perror("eroare la scrierea in fisierul de logare");
        close(treasureFd);
        close(targetLoggingFd);
        return;
    };
    if(close(treasureFd) < 0) {
        perror("eroare la inchiderea fisierului treasures");
        return;
    }
    if(closedir(dir) < 0) {
        perror("eroare la inchiderea directorului");
        return;
    }
    if(close(targetLoggingFd) < 0) {
        perror("eroare la inchiderea fisierului de logare");
        return;
    }
    putchar('\n');
}

void list(char *hunt) {
    char targetLoggingFile[150];
    char filepath[150];
    sprintf(filepath, "%s/treasures", hunt);
    sprintf(targetLoggingFile, "%s/logging-file.txt", hunt);
    int targetLoggingFd = open(targetLoggingFile, O_WRONLY | O_APPEND, 0644);
    if(targetLoggingFd < 0) {
        perror("eroare la deschiderea fisierului de logare");
        return;
    }
    int treasureFd = open(filepath, O_RDONLY);
    if(treasureFd < 0) {
        perror("acest hunt nu exista");
        close(targetLoggingFd);
        return;
    }
    Treasure tr;
    printf("Lista comorilor de la huntul %s:\n", hunt);
    ssize_t bytesRead;
    while((bytesRead = read(treasureFd, &tr, sizeof(Treasure))) == sizeof(Treasure)) {
        showTreasure(&tr);
    }
    if(bytesRead < 0) {
        perror("eroare la citirea din fisierul treasures");
        close(treasureFd);
        close(targetLoggingFd);
        return;
    }
    char logMessage[100];
    int len = sprintf(logMessage, "S a afisat lista comorilor de la huntul %s\n", hunt);
    if(write(targetLoggingFd, logMessage, len) != len) {
        perror("eroare la scrierea in fisierul de logare");
    }
    
    if(close(treasureFd) < 0) {
        perror("eroare la inchiderea fisierului treasures");
    }
    
    if(close(targetLoggingFd) < 0) {
        perror("eroare la inchiderea fisierului de logare");
    }
    putchar('\n');
}

void view(char *hunt, int id) {
    char filepath[150];
    sprintf(filepath, "%s/treasures", hunt);
    char targetLoggingFile[150];
    sprintf(targetLoggingFile, "%s/logging-file.txt", hunt);
    int targetLoggingFd = open(targetLoggingFile, O_WRONLY | O_APPEND, 0644);
    if(targetLoggingFd < 0) {
        perror("eroare la deschiderea fisierului de logare");
        return;
    }
    int treasureFd = open(filepath, O_RDONLY);
    if(treasureFd < 0) {
        perror("acest hunt nu exista");
        close(targetLoggingFd);
        return;
    }
    Treasure tr;
    int found = 0;
    ssize_t bytesRead;
    
    while((bytesRead = read(treasureFd, &tr, sizeof(Treasure))) == sizeof(Treasure)) {
        if(tr.id == id) {
            showTreasure(&tr);
            found = 1;
            break;
        }
    }
    if(bytesRead < 0) {
        perror("eroare la citirea din fisierul treasures");
        close(treasureFd);
        close(targetLoggingFd);
        return;
    }
    char logMessage[100];
    int len;
    if(found) {
        len = sprintf(logMessage, "S a afisat comoara cu id-ul %d\n", id);
    } else {
        len = sprintf(logMessage, "Comoara cu id-ul %d nu exista\n", id);
    }
    if(write(targetLoggingFd, logMessage, len) != len) {
        perror("eroare la scrierea in fisierul de logare");
    }
    if(close(treasureFd) < 0) {
        perror("eroare la inchiderea fisierului treasures");
    }
    if(close(targetLoggingFd) < 0) {
        perror("eroare la inchiderea fisierului de logare");
    }
    putchar('\n');
}

void remove_hunt(char *hunt) {
    char filepath[150];
    sprintf(filepath, "%s/treasures", hunt);
    char targetLoggingFile[150];
    sprintf(targetLoggingFile, "%s/logging-file.txt", hunt);
    char originalLoggingFile[150];
    sprintf(originalLoggingFile, "logging-file-%s.txt", hunt);
    if(unlink(originalLoggingFile) < 0) {
        perror("eroare la stergerea legaturii simbolice");
    }
    if(remove(filepath) < 0) {
        perror("eroare la stergerea fisierului treasures");
        return;
    }
    if(remove(targetLoggingFile) < 0) {
        perror("eroare la stergerea fisierului de logare");
        return;
    }
    if(rmdir(hunt) < 0) {
        perror("eroare la stergerea directorului");
        return;
    }
    putchar('\n');
}

void remove_treasure(char *hunt, int id) {
    char targetLoggingFile[150];
    sprintf(targetLoggingFile, "%s/logging-file.txt", hunt);
    int targetLoggingFd = open(targetLoggingFile, O_WRONLY | O_APPEND, 0644);
    if(targetLoggingFd < 0) {
        perror("eroare la deschiderea fisierului de logare");
        return;
    }
    char filepath[150];
    sprintf(filepath, "%s/treasures", hunt);
    int treasureFd = open(filepath, O_RDONLY);
    if(treasureFd < 0) {
        perror("eroare la deschiderea fisierului treasures");
        close(targetLoggingFd);
        return;
    }
    Treasure tr;
    int tempFd = open("temp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(tempFd < 0) {
        perror("eroare la deschiderea fisierului temporar");
        close(treasureFd);
        close(targetLoggingFd);
        return;
    }
    int found = 0;
    ssize_t bytesRead;
    while((bytesRead = read(treasureFd, &tr, sizeof(Treasure))) == sizeof(Treasure)) {
        if(tr.id == id) {
            found = 1;
            continue;
        }
        
        if(write(tempFd, &tr, sizeof(Treasure)) != sizeof(Treasure)) {
            perror("eroare la scrierea in fisierul temporar");
            close(treasureFd);
            close(tempFd);
            close(targetLoggingFd);
            unlink("temp");
            return;
        }
    }
    if(bytesRead < 0) {
        perror("eroare la citirea din fisierul treasures");
        close(treasureFd);
        close(tempFd);
        close(targetLoggingFd);
        unlink("temp");
        return;
    }
    close(treasureFd);
    close(tempFd);
    
    char logMessage[100];
    int len;
    if(!found) {
        printf("Comoara cu id-ul %d nu exista\n", id);
        len = sprintf(logMessage, "Comoara cu id-ul %d nu exista\n", id);
        unlink("temp"); 
    } else {
        len = sprintf(logMessage, "Comoara cu id-ul %d a fost stearsa\n", id);
        if(unlink(filepath) < 0) {
            perror("eroare la stergerea fisierului treasures");
            close(targetLoggingFd);
            return;
        }
        if(rename("temp", filepath) < 0) {
            perror("eroare la redenumirea fisierului temporar");
            close(targetLoggingFd);
            return;
        }
    }
    
    if(write(targetLoggingFd, logMessage, len) != len) {
        perror("eroare la scrierea in fisierul de logare");
    }
    
    close(targetLoggingFd);
    putchar('\n');
}

int main(int argc,char **argv) {
    if(argc < 2) {
        printf("Utilizare: %s <comanda> [<parametru>]\n", argv[0]);
        return 1;
    }
    if(!strcmp(argv[1], "add")) {
        if(argc < 3) {
            printf("Utilizare: %s add <hunt>\n", argv[0]);
            return 1;
        }
        add(argv[2]);
    } else if(!strcmp(argv[1], "list")) {
        if(argc < 3) {
            printf("Utilizare: %s list <hunt>\n", argv[0]);
            return 1;
        }
        list(argv[2]);
    } else if(!strcmp(argv[1], "view")) {
        if(argc < 4) {
            printf("Utilizare: %s view <hunt> <id>\n", argv[0]);
            return 1;
        }
        view(argv[2], atoi(argv[3]));
    } else if(!strcmp(argv[1], "remove_hunt")) {
        if(argc < 3) {
            printf("Utilizare: %s remove_hunt <hunt>\n", argv[0]);
            return 1;
        }
        remove_hunt(argv[2]);
    } else if(!strcmp(argv[1], "remove_treasure")) {
        if(argc < 4) {
            printf("Utilizare: %s remove_treasure <hunt> <id>\n", argv[0]);
            return 1;
        }
        remove_treasure(argv[2], atoi(argv[3]));
    } else {
        printf("Comanda necunoscuta: %s\n", argv[1]);
        return 1;
    }
    return 0;
}
