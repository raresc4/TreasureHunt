#include <stdio.h>   
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

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
    Treasure treasure;
    scanf("%d", &treasure.id);
    scanf("%s", treasure.username);
    scanf("%d", &treasure.coordonate.x);
    scanf("%d", &treasure.coordonate.y);
    scanf("%s", treasure.clue);
    scanf("%d", &treasure.value);
    return treasure;
}

void showTreasure(Treasure *tr) {
    printf("id: %d\nusername: %s\nx coordonate: %d\ny coordonate: %d\nclue: %s\nvalue: %d\n", tr->id, tr->username, tr->coordonate.x,tr->coordonate.y,tr->clue,tr->value);
}

void add(char *hunt) {
    DIR *dir = opendir(hunt);
    char newPath[100];
    int new = 0;
    if(!dir) {
        new = 1;
        sprintf(newPath,"./%s",hunt);
        if(mkdir(newPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
            perror("eroarea la crearea noului hunt");
            return;
        }
    }
    // struct dirent *fisier = NULL;
    // int exista = 0;
    // Treasure tr = createTreasure();
    // char name[100];
    // sprintf(name,"hunt%d",tr.id);
    // FILE *f = fopen('')
    if(closedir(dir) < 0) {
        perror("eroare la inchiderea directorului");
        return;
    }
}

int main(int argc,char **argv) {
add("hunt2");
return 0;
}