#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
 
/*
 * L'obiettivo della verifica è la parallelizzazione della somma di una sequenza
 * di numeri interi. Viene fornito un programma 'sum' che esegue un numero
 * casuale di istanze del programma 'somma' che noi dobbiamo implementare.
 * Ciascuna istanza riceve su una pipe una stringa contenente una sequenza di
 * numeri interi non negativi separati da virgola e deve scrivere la somma di
 * questi numeri (sempre come stringa) all'interno di una seconda pipe.
 * I path delle due pipe da utilizzare sono passati al nostro programma quando
 * viene eseguito da 'sum'.
 */
int main(int argc, char *argv[]) {
    int fd_in, fd_out, sum, n;
    char c, *str_sum;
    size_t str_size;
 
    /* Controllo che i path delle pipe siano stati forniti al programma. */
    if (argc != 3) {
        fprintf(stderr, "Utilizzo: ./somma <pipe_in> <pipe_out>");
        exit(EXIT_FAILURE);
    }
 
    /* Apro le pipe. Termina il programma in caso di errore. */
    fd_in = open(argv[1], O_RDONLY);
    if (fd_in < 0) {
        fprintf(stderr, "Non riesco ad aprire '%s' in lettura.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    fd_out = open(argv[2], O_WRONLY);
    if (fd_out < 0) {
        fprintf(stderr, "Non riesco ad aprire '%s' in lettura.\n", argv[2]);
        exit(EXIT_FAILURE);
    }

     /* Leggi dalla pipe un carattere per volta. Se il carattere letto è:
     * - una cifra, costruisci il numero corrente in maniera incrementale;
     * - una virgola, il numero è stato letto e può essere aggiunto alla somma
     *   parziale;
     * - un altro carattere, si tratta di un errore.
     * Il ciclo termina la funzione read ritorna zero, ovvero quando la pipe
     * è vuota ed è stata chiusa in lettura dal programma 'sum', quindi la
     * sequenza di numeri da leggere è terminata. */

    sum = n = 0;
    while(read(fd_in, &c, sizeof(char))) {
        if(c >= '0' && c<= '9') {
            n = n * 10 + (c - '0');
        }
        else if (c == ',') {
            sum += n;
            n = 0;
        }
        else {
        fprintf(stderr, "Carattere non atteso: %c\n", c);
        exit(EXIT_FAILURE);
        }     
    }
    close(fd_in);

    /* Alloca una stringa di dimensione sufficiente per rappresentare il
    * risultato ed invialo sulla pipe. Chiudi la pipe una volta effettuata
    * la scrittura. */

    str_size = snprintf(NULL, 0, "%d", sum) + 1;
    str_sum = malloc(sizeof(char) * str_size);
    snprintf(str_sum, str_size, "%d", sum);
    write(fd_out, str_sum, str_size);
    free(str_sum);
    close(fd_out);

    return EXIT_SUCCESS;
}