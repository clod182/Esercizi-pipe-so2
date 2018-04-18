/*Welcome to pipe-pong, il gioco del ping-pong con le pipe!

1) La posizione orizzontale x della pallina viene inviata sulle pipe
   pipePongOut nel formato "x," (la virgola fa da separatore)
2) Per iniziare a giocare devi inviare "g" su pipePongIn
3) Devi inviare su pipePongIn il carattere "l" per spostare la tua
   racchetta a sinistra o il carattere "r" per spostarla a destra
4) La posizione iniziale della racchetta è 10

Ricorda che se vuoi giocare da terminale puoi fare "cat > pipePongIn"
e inviare da tastiera i comandi. Con "cat pipePongOut" puoi osservare
quello che sto scrivendo

Devi scrivere un programma che riesca a giocare!!

Scrivi 'g' su pipePongIn per giocare
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
 
#define PIPEIN "pipePongIn"
#define PIPEOUT "pipePongOut"
#define BUFSIZE 10
#define LEFT "l"
#define RIGHT "r"
 
int main() {
    int i, fd_in, fd_out, ballx, position, num_moves;
    char stop, buffer[BUFSIZE], *direction;
 
    /* Crea le due pipe (se già non esistono) ed apri:
     * - pipePongIn in scrittura, per inviare le mosse all'altro programma;
     * - pipePongOut in lettura, per leggere la posizione corrente della pallina. */

    mkfifo(PIPEOUT, 0666);
    mkfifo(PIPEIN, 0666);
    fd_in = open(PIPEOUT, O_RDONLY);
    fd_out = open(PIPEIN, O_WRONLY);
    if (fd_in < 0 || fd_out < 0) {
        perror("Errore nell'apertura delle pipe");
        exit(EXIT_FAILURE);
    }

    /* Flag usata per la terminazione della partita. */
    stop = 0;
    /* La posizione iniziale della racchetta è 10, al centro del campo. */
    position = 10;
 
    /* Inizia la partita! */
    write(fd_out, "g", 1);

    while(!stop) {
        /* Leggi la posizione della pallina. */
        i = 0;
        while(i < BUFSIZE && read(fd_in, buffer+i, 1) && buffer[i] != ',') {
            i++;
        }
        if(i == 0){
            stop = 1;
        }
        else if(i >= BUFSIZE) {
            /* Qui non ci dovremmo mai arrivare, ma è sempre bene gestire eventuali overflow. */
            fprintf(stderr, "Non dovrei essere qui. BOOM!\n");
            exit(EXIT_FAILURE);
        }
        else {
            buffer[i+1] = '\0';
            //converto la posizone della pallina
            sscanf(buffer, "%d", &ballx);

            /* Vai a sinistra o a destra del numero di mosse necessarie per muovere la
             * racchetta nella stessa posizione della pallina. */
             direction = ballx < position ? LEFT : RIGHT;
             num_moves = abs(ballx - position);
             while(num_moves > 0){
                 write(fd_out, direction, 1);
                 num_moves--;
             }
             position = ballx;
        }
    }
    /* Chiudi entrambe le pipe (la rimozione dal filesystem con 'unlink' viene già
     * fatta dall'altro programma). */
    close(fd_in);
    close(fd_out);
 
    return EXIT_SUCCESS;
}
