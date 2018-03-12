#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#define WR_PIPE "carPipeIn"
#define RD_PIPE "carPipeOut"
#define WIDTH 15
#define ROUNDS 20
#define LEFT 'l'
#define RIGHT 'r'
 
/*
L'obiettivo del programma è di fornire al programma di test la sequenza di mosse necessarie
affinché la macchina raggiunga la fine del percorso evitando tutti gli ostacoli e senza uscire
di strada utilizzando le pipe come canale di comunicazione.
Per fare ciò, viene iterato per 20 volte (pari al numero di round) il seguente schema:
- leggi dalla pipe "carPipeOut" la posizione del passaggio attraverso il quale far passare la
  macchina;
- calcola la direzione dello spostamento ('l' o 'r') e il numero di mosse da fare;
- crea la stringa corrispondente alle mosse da fare e scrivila in "carPipeIn".
*/
 
int main() {
    int fw, fr, carPos = WIDTH / 2, holePos, i, j, numMoves;
    char buffer[WIDTH], direction;
    /*apro le pipe*/
    mkfifo(WR_PIPE, 0666);
    mkfifo(RD_PIPE, 0666);
    fw = open(WR_PIPE, O_WRONLY);
    fr = open(RD_PIPE, O_RDONLY);
    if (fw < 0 || fw < 0) {
        perror("Errore nell'apertura delle pipe");
        exit(EXIT_FAILURE);
    }
    i=0;
    //gioco la partita per arrivare ai 20 round richiesti
    while(i < ROUNDS) {
        j=0;
        /* Leggo la posizione corrente dell'ostacolo dalla pipe e prendo il
        valore numerico corrispondente. */
        while(j < WIDTH && read(fr, buffer+j, 1) && buffer[j] != '#') {
            j++;
        }
        sscanf(buffer, "%d#", &holePos);
        numMoves = abs(carPos - holePos);
        /* Se occorre spostarsi, determina la direzione di spostamento (data dal segno
        della differenza tra la posizione corrente e quella del passaggio), crea la
        stringa contenente le mosse da fare e inviala nella pipe. Aggiorna anche la
        posizione della macchina. */
        if(numMoves != 0) {
            direction = (carPos < holePos) ? RIGHT : LEFT;
            while(numMoves > 0){
                write(fw, &direction, 1);
                numMoves--;
            }
            carPos = holePos;
        }


        i++;
    }
    /* Chiudi le pipe. */
    close(fw);
    close(fr);
 
    return EXIT_SUCCESS;
}