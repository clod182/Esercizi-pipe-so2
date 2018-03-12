#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#define WR_PIPE "shotPipeIn"
#define RD_PIPE "shotPipeOut"
#define BUFFSIZE 10
#define ROUNDS 10
#define LEFT 'l'
#define RIGHT 'r'

int main() {
    int fw, fr, mirinox, mirinoy, bersx, bersy, i, j, numMoves;
    char buffer[BUFFSIZE], direction;
    /*apro le pipe*/
    mkfifo(WR_PIPE, 0666);
    mkfifo(RD_PIPE, 0666);
    fw = open(WR_PIPE, O_WRONLY);
    fr = open(RD_PIPE, O_RDONLY);
    if (fw < 0 || fw < 0) {
        perror("Errore nell'apertura delle pipe");
        exit(EXIT_FAILURE);
    }

    while(i < ROUNDS) {
        /*j=0;
        while(j < BUFFSIZE && read(fr, buffer + j, 1) && buffer[j] != '#') {
            j++;
        }*/
        read(fr, &buffer, BUFFSIZE);
        sscanf(buffer, "%d,%d#", &bersx, &bersy);
        sprintf(buffer, "%d,%d#", bersx - mirinox, bersy - mirinoy);
        write(fw, buffer, strlen(buffer));        
        mirinox = bersx;
        mirinoy = bersy;
        i++;
    }
    close(fw);
    close(fr);

    return EXIT_SUCCESS;
}