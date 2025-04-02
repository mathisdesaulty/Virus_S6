/*
Installer :
    sudo apt update
    sudo apt install libgtk-4-dev
    sudo apt autoremove --purge snapd
    sudo apt install libgtk-3-dev pkg-config

Compiler : 
    gcc -o cesar cesar.c $(pkg-config --cflags --libs gtk+-3.0)
    ./cesar
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *converseToCesar(char *mot, int index) {
    char *result = strdup(mot);
    for (int i = 0; result[i] != '\0'; i++) {
        if (isalpha(result[i])) {
            char base = isupper(result[i]) ? 'A' : 'a';
            result[i] = base + ((result[i] - base + index) % 26);
        }
    }
    return result;
}

int main() {
    char message[512]; 
    int key;

    printf("Entrez votre message : ");
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = 0;

    printf("Entrez la clé : ");
    scanf("%d", &key);

    key = key % 26;
    if (key < 0) {
        key += 26;
    }

    char *code = converseToCesar(message, key);

    printf("Code : %s\n", code);

    free(code);

    return 0;
}