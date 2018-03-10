#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#define PIPEIN "/tmp/pipeSnakeIn"
#define PIPEOUT "/tmp/pipeSnakeOut"
#define BUFSIZE 16
 
/*
 * Lo scopo della verifica è realizzare un programma in grado di giocare in maniera
 * automatica ad una versione semplificata di Snake, dove il serpente è di lunghezza
 * tale per cui sia impossibile che collida su sè stesso e i muri possono essere
 * attraversati senza perdere la partita.
 * Il programma fornito riceve i comandi di inizio partita e cambiamento della direzione
 * su PIPEIN e scrive su PIPEOUT la distanza (numero floating point sotto forma di stringa)
 * tra la testa del serpente e il cibo.
 * I cambiamenti di direzione vengono fatti in due casi:
 * - quando la distanza dal cibo aumenta rispetto alla lettura precedente;
 * - quando la distanza è 1, condizione necessaria affinché il serpente non ruoti intorno
 *   al cibo all'infinito senza mai mangiarlo.
 * Il programma termina quando la read restituisce zero, ovvero la pipe è vuota ed è stata
 * chiusa in scrittura dall'altro processo. Questo accade quando la partita è stata vinta.
 */
 
int main() {
    int i, fd_in, fd_out;
    float prev_distance, distance;
    char won, overflow, buffer[BUFSIZE];

    /* Crea le pipe (se non esistono) ed aprile. Termina se si verifica un errore durante
     * l'apertura di una delle due. */
    mkfifo(PIPEOUT, 0666);
    mkfifo(PIPEIN, 0666);
    fd_in = open(PIPEOUT, O_RDONLY);
    if (fd_in < 0) {
        fprintf(stderr, "Non riesco ad aprire '%s' in lettura.\n", PIPEOUT);
        exit(EXIT_FAILURE);
    }
    fd_out = open(PIPEIN, O_WRONLY);
    if (fd_out < 0) {
        fprintf(stderr, "Non riesco ad aprire '%s' in scrittura.\n", PIPEIN);
        exit(EXIT_FAILURE);
    }

    /* Inizializza la distanza precedente con un valore arbitrario. La scelta è irrilevante,
     * al massimo viene fatta una mossa in più ad inizio partita. */
    prev_distance = 100;
    /* Inizia la partita inviando il carattere 'S' nella pipe. */
    write(fd_out, "S", 1);

    /* inizializzo le condizioni di partita*/
    won = overflow = 0;
    while (!won && !overflow) {

        /* Copia il contenuto della pipe nel buffer finché non si verifica una delle seguenti
         * condizioni:
         * - i >= BUFSIZE: overflow, ma non dovrebbe mai accadere;
         * - read restituisce zero: in questo caso la partita è terminata;
         * - è stato letto il separatore ','. */
        i = 0;
        while(i < BUFSIZE && read(fd_in, buffer + i, 1) && buffer[i] != ','){
            i++;
        }

        if (i == 0) {
            /* Dal momento che non possiamo avere due virgole consecutive nella pipe, ci
             * ritroviamo in questo caso quando read ha restituito 0, ovvero la partita
             * è terminata. */
            won = 1;
        } 
        else if (i >= BUFSIZE) {
            /* Per sicurezza, evitiamo gli overflow... */
            overflow = 1;
        } 
        else {
            /* Converti la distanza, espressa come stringa, in un numero floating point. */
            sscanf(buffer, "%f", &distance);
            /* Invia una mossa se si verifica una delle condizioni descritte nel commento iniziale.
             * Quando si usano i numeri in virgola mobile è preferibile evitare le uguaglianze (vedi
             * https://it.wikipedia.org/wiki/Numero_in_virgola_mobile#Problemi_con_l.27uso_della_virgola_mobile
             * per maggiori dettagli). */
             if (distance <= 1.005 || distance >= prev_distance) {
                write(fd_out, "<", 1);
            }
            /* Aggiorna la distanza precedente. */
            prev_distance = distance;
        }
    }
    /* Chiudi le pipe e stampa un messaggio di errore in caso di overflow. La rimozione delle pipe
     * dal filesystem viene fatta dall'altro programma. */
    close(fd_in);
    close(fd_out);
    if (overflow) {
        fprintf(stderr, "Overflow? Non dovrebbe mai succedere!\n");
        return EXIT_FAILURE;
    }
 
    return EXIT_SUCCESS;
}


