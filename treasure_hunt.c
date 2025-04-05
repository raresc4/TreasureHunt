#include <stdio.h>   
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

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
    char absoluteTargetPath[406];
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
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("eroare la obtinerea directorului curent");
        return;
    }
    sprintf(absoluteTargetPath, "%s/%s", cwd, targetLoggingFile);
    FILE *targetLoggingFilePtr = fopen(targetLoggingFile, "at");
    if(!targetLoggingFilePtr) {
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
            fclose(targetLoggingFilePtr);
            return;
        }
    }
    char filepath[150];
    if(new) {
        sprintf(filepath, "%s/%s", newDirPath, "treasures");
    } else {
        sprintf(filepath, "%s/treasures", hunt);    
    }
    FILE *f = fopen(filepath, "ab+");
    if(!f) {
        perror("eroare la deschiderea sau crearea fisierului treasures");
        return;
    }
    Treasure tr = createTreasure();
    if(fwrite(&tr, sizeof(Treasure), 1, f) != 1) {
        perror("eroare la scrierea in fisierul treasures");
        return;
    }
    fprintf(targetLoggingFilePtr, "Adaugata comoara cu id-ul %d\n", tr.id);
    if(fclose(f)) {
        perror("eroare la inchiderea fisierului treasures");
        return;
    }
    if(closedir(dir) < 0) {
        perror("eroare la inchiderea directorului");
        return;
    }
    if(fclose(targetLoggingFilePtr)) {
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
    FILE *targetLoggingFilePtr = fopen(targetLoggingFile, "at");
    if(!targetLoggingFilePtr) {
        perror("eroare la deschiderea fisierului de logare");
        return;
    }
    FILE *f = fopen(filepath, "rb");
    if(!f) {
        perror("acest hunt nu exista");
        return;
    }
    Treasure tr;
    printf("Lista comorilor de la huntul %s:\n", hunt);
    while(fread(&tr, sizeof(Treasure), 1, f)) {
        showTreasure(&tr);
    }
    fprintf(targetLoggingFilePtr, "S a afisat lista comorilor de la huntul %s\n", hunt);
    if(fclose(f)) {
        perror("eroare la inchiderea fisierului treasures");
        return;
    }
    putchar('\n');
}

void view(char *hunt, int id) {
    char filepath[150];
    sprintf(filepath, "%s/treasures", hunt);
    char targetLoggingFile[150];
    sprintf(targetLoggingFile, "%s/logging-file.txt", hunt);
    FILE *targetLoggingFilePtr = fopen(targetLoggingFile, "at");
    if(!targetLoggingFilePtr) {
        perror("eroare la deschiderea fisierului de logare");
        return;
    }
    FILE *f = fopen(filepath, "rb");
    if(!f) {
        perror("acest hunt nu exista");
        return;
    }
    Treasure tr;
    int found = 0;
    while(fread(&tr, sizeof(Treasure), 1, f)) {
        if(tr.id == id) {
            showTreasure(&tr);
            found = 1;
            break;
        }
    }
    if(found) {
        fprintf(targetLoggingFilePtr, "Comoara cu id-ul %d a fost vizualizata\n", id);
    } else {
        fprintf(targetLoggingFilePtr, "Comoara cu id-ul %d nu exista\n", id);
    }
    if(!found) {
        printf("Comoara cu id ul %d nu exista\n", id);
    }
    if(fclose(f)) {
        perror("eroare la inchiderea fisierului treasures");
        return;
    }
    if(fclose(targetLoggingFilePtr)) {
        perror("eroare la inchiderea fisierului de logare");
        return;
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
    FILE *targetLoggingFilePtr = fopen(targetLoggingFile, "a1t");
    if(!targetLoggingFilePtr) {
        perror("eroare la deschiderea fisierului de logare");
        return;
    }
    char filepath[150];
    sprintf(filepath, "%s/treasures", hunt);
    FILE *f = fopen(filepath, "rb+");
    if(!f) {
        perror("eroare la deschiderea fisierului treasures");
        return;
    }
    Treasure tr;
    FILE *temp = fopen("temp", "wb+");
    if(!temp) {
        perror("eroare la deschiderea fisierului temporar");
        fclose(f);
        return;
    }
    int found = 0;
    while(fread(&tr, sizeof(Treasure), 1, f)) {
        if(tr.id == id) {
            found = 1;
            continue;
        }
        if(fwrite(&tr, sizeof(Treasure), 1, temp) != 1) {
            perror("eroare la scrierea in fisierul temporar");
            fclose(f);
            fclose(temp);
            return;
        }
    }
    if(!found) {
        printf("Comoara cu id ul %d nu exista\n", id);
        if(fclose(f)) {
            perror("eroare la inchiderea fisierului treasures");
            fclose(temp);
            return;
        }
        if(fclose(temp)) {
            perror("eroare la inchiderea fisierului temporar");
            return;
        }
        fprintf(targetLoggingFilePtr, "Comoara cu id-ul %d nu exista\n", id);
        remove("temp");
        return;
    }
    fprintf(targetLoggingFilePtr, "Comoara cu id-ul %d a fost stearsa\n", id);
    if(fclose(f)) {
        perror("eroare la inchiderea fisierului treasures");
        fclose(temp);
        return;
    }
    if(fclose(temp)) {
        perror("eroare la inchiderea fisierului temporar");
        return;
    }
    if(fclose(targetLoggingFilePtr)) {
        perror("eroare la inchiderea fisierului de logare");
        return;
    }
    if(remove(filepath) < 0) {
        perror("eroare la stergerea fisierului treasures");
        return;
    }
    if(rename("temp", filepath) < 0) {
        perror("eroare la redenumirea fisierului temporar");
        return;
    }
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
