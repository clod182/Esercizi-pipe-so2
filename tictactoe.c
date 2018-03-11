/*Welcome to pipe-tac-toe (tris con le pipe)!

1) Le mosse vengono inviate sulle pipe ticPipeOut e ticPipeIn
   nel formato x,y#
2) Per iniziare a giocare devi inviare 'g' su ticPipeIn
3) Dopo ogni mossa viene inviato su ticPipeOut il carattere '&'
   se la partita va avanti, oppure '$' se uno dei giocatori
   ha vinto

Ricorda che se vuoi giocare da terminale puoi fare cat > ticPipeIn
e inviare da tastiera le mosse. Con cat ticPipeOut puoi osservare
quello che sto scrivendo

Devi scrivere un programma che riesca a vincere per 100 volte!
Non è un esercizio di algoritmi, per vincere ... basta giocare!

... in bocca al lupo!

Scrivi 'g' su ticPipeIn per giocare
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
 
#define WR_PIPE "ticPipeIn"
#define RD_PIPE "ticPipeOut"
#define BUFSIZE 5
#define VICTORIES 100
#define SIZE 3
#define START "g"
#define END '$'

/* 
 * La verifica richiede la realizzazione di un programma che vinca
 * 100 partite (non necessariamente consecutive) a tris contro il
 * computer.
 * 
 * Una partita viene avviata scrivendo il carattere 'g' nella pipe
 * "ticPipeIn". Il programma riceve in "ticPipeOut" la posizione
 * occupata dalla pedina dell'avversario nel formato "R,C#", dove R
 * e C corrispondono al numero di riga e colonna. Riceve inoltre un
 * carattere di controllo per determinare lo stato della partita:
 * '$' indica che è terminata, '&' che non lo è. Nel secondo caso,
 * il programma scrive in "ticPipeIn" la mossa da fare, usando il
 * medesimo formato, e legge da "ticPipeOut" lo stato della partita
 * in seguito alla sua mossa.
 */
 
int main(int argc, char *argv[]) {
    int rp, wp, win, i, row, col;
    char board[SIZE][SIZE], buffer[BUFSIZE], status;
 
    /* Apri le pipe: termina il programma in caso di errore. */
    wp = open(WR_PIPE, O_WRONLY);
    if (wp < 0) {
        fprintf(stderr, "Non riesco ad aprire '%s' in scrittura.\n", WR_PIPE);
        exit(EXIT_FAILURE);
    }
    rp = open(RD_PIPE, O_RDONLY);
    if (rp < 0) {
        fprintf(stderr, "Non riesco ad aprire '%s' in lettura.\n", RD_PIPE);
        exit(EXIT_FAILURE);
    }
        /* Imposta il seed per il generatore di numeri casuali. */
    srand(time(NULL));
 
    win = 0;
    while (win < VICTORIES) {
        /* Pulisci il campo ed inizia una nuova partita! */
        memset(board, 0, SIZE * SIZE);
        //inizio il game
        write(wp, START, strlen(START));
        do {
            /* Leggi la posizione dell'avversario e segnala come occupata. */
            i = 0;
            while(read(rp,buffer + i, 1) && buffer[i] != '#') {
                i++;
            }
            buffer[i+1] = '\0';
            //converto il contenuto del buffer nel formato che mi serve
            sscanf(buffer, "%d,%d#", &row,&col);
            //riempio il campo con la mossa dell'avversario
            board[row][col] = 1;        

            /* Leggi lo stato della partita. Se è terminata, abbiamo una sconfitta o
            * un pareggio. Altrimenti facciamo la nostra mossa! */
            read(rp, &status, 1);
            if (status != END) {
                /* Scegliamo in maniera casuale riga
                * e colonna finché non viene trovata una posizione libera. */
                do {
                    row = rand() % SIZE;
                    col = rand() % SIZE;
                } while(board[row][col] != 0);
                /* Scrivi la posizione sulla pipe e segnala come occupata. */
                snprintf(buffer, BUFSIZE, "%d,%d#", row, col);
                write(wp, buffer, strlen(buffer));
                board[row][col] = 1;

                read(rp, &status, 1);
                if (status == END) {
                    win++;
                }
            }
        } while(status != END);
    }
    //quando abbiamo raggiunto le win richieste il nostro compito è finito
    close(wp);
    close(rp);

    return EXIT_SUCCESS;
}



