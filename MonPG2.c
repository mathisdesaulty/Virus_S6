/*

gcc -o jeu jeu.c `pkg-config --cflags --libs gtk+-3.0` -lm

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int nb_devine;   
    int nb_tentatives; 
} GameData;

void play_game(GameData *data) {
    int guess;
    char play_again;

    do {
        data->nb_devine = 1 + rand() % 100;
        data->nb_tentatives = 0;

        printf("Devine un nombre entre 1 et 100 !\n");

        while (1) {
            printf("Entre ton nombre : ");
            scanf("%d", &guess);
            data->nb_tentatives++;

            if (guess < data->nb_devine) {
                printf("Trop bas ! Tentatives : %d\n", data->nb_tentatives);
            } else if (guess > data->nb_devine) {
                printf("Trop haut ! Tentatives : %d\n", data->nb_tentatives);
            } else {
                printf("Bravo ! Trouvé en %d tentatives !\n", data->nb_tentatives);
                break;
            }
        }

        printf("Rejouer ? (o/n) : ");
        scanf(" %c", &play_again);

    } while (play_again == 'o' || play_again == 'O');
}

int main() {

    srand(time(NULL));

    GameData data = { .nb_devine = 0, .nb_tentatives = 0 };

    play_game(&data);

    return 0;
}