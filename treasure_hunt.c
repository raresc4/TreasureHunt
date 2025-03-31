#include <stdio.h>   
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

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
    scanf("%d", &treasure.coordonate.x);
    scanf("%d", &treasure.coordonate.y);
    printf("Introdu indiciul:\n");
    scanf("%s", treasure.clue);
    printf("Introdu valoarea:\n");
    scanf("%d", &treasure.value);
    return treasure;
}

void showTreasure(Treasure *tr) {
    printf("id: %d\nusername: %s\nx coordonate: %d\ny coordonate: %d\nclue: %s\nvalue: %d\n", tr->id, tr->username, tr->coordonate.x,tr->coordonate.y,tr->clue,tr->value);
}

void add(char *hunt) {
    DIR *dir = opendir(hunt);
    char newDirPath[100];
    int new = 0;
    if(!dir) {
        new = 1;
        sprintf(newDirPath,"./%s",hunt);
        if(mkdir(newDirPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
            perror("eroarea la crearea noului hunt");
            return;
        }
    }
    if(new) {
        dir = opendir(newDirPath);
        if(!dir) {
            perror("eroare la deschiderea directorului nou creat");
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
    if(fclose(f)) {
        perror("eroare la inchiderea fisierului treasures");
        return;
    }
    if(closedir(dir) < 0) {
        perror("eroare la inchiderea directorului");
        return;
    }
    putchar('\n');
}

void list(char *hunt) {
    char filepath[150];
    sprintf(filepath, "%s/treasures", hunt);
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
    if(fclose(f)) {
        perror("eroare la inchiderea fisierului treasures");
        return;
    }
    putchar('\n');
}

void view(char *hunt, int id) {
    char filepath[150];
    sprintf(filepath, "%s/treasures", hunt);
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
    if(!found) {
        printf("Comoara cu id ul %d nu exista\n", id);
    }
    if(fclose(f)) {
        perror("eroare la inchiderea fisierului treasures");
        return;
    }
    putchar('\n');
}

void remove_hunt(char *hunt) {
    char filepath[150];
    sprintf(filepath, "%s/treasures", hunt);
    if(remove(filepath) < 0) {
        perror("eroare la stergerea fisierului treasures");
        return;
    }
    if(rmdir(hunt) < 0) {
        perror("eroare la stergerea directorului");
        return;
    }
    putchar('\n');
}

void remove_treasure(char *hunt, int id) {
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
        remove("temp");
        return;
    }
    if(fclose(f)) {
        perror("eroare la inchiderea fisierului treasures");
        fclose(temp);
        return;
    }
    if(fclose(temp)) {
        perror("eroare la inchiderea fisierului temporar");
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
//add("hunt2");
remove_treasure("hunt2", 2);
list("hunt2");
//remove_hunt("hunt2");
return 0;
}
