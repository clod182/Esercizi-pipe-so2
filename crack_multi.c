#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
 
/*
 * La verifica consiste nel parallelizzare il cracking di hash SHA-256 su più
 * processi e stampare una statistica sul numero di hash crackati con successo
 * rispetto al numero di hash totali. Gli hash vengono forniti al programma
 * tramite command-line.
 * 
 * Abbiamo a disposizione un programma 'cracker' che si occupa di fare il
 * cracking cercando se una parola presente nel dizionario ha un hash uguale a
 * quello fornito via standard input. Il programma termina con valore di
 * ritorno 0 se l'operazione ha successo, 1 altrimenti.
 * 
 * Per ciascun hash da crackare:
 * - creiamo una pipe senza nome e facciamo una fork;
 * - il processo padre scrive nella pipe l'hash da crackare;
 * - il processo figlio associa al descrittore 0 (che identifica lo standard
 *   input) l'estremità della pipe aperta in lettura, quindi esegue il programma
 *   'cracker' che proverà a crackare l'hash fornito dal padre.
 * Dopo aver creato tutti i figli, il padre ne attende la terminazione ed
 * incrementa il contatore degli hash crackati quando il codice di ritorno del
 * figlio è pari a 0. Notare inoltre come le pipe vengano chiuse immediatamente
 * dai processi quando non sono più necessarie.
 * 
 * NB. Non era necessaria la gestione dell'errore nella fork/exec, ma per
 * completezza consideriamo anche questa possibilità. In particolare, nel caso
 * della fork, il processo padre e tutti i figli creati fino al momento del
 * fallimento vengono terminati (la pipe aperta prima della fork viene chiusa
 * automaticamente alla terminazione del processo). Nel caso della exec, il
 * figlio termina con codice di errore 1 (che identifica un fallimento).
 */
 
int main(int argc, char *argv[]) {
    int success, total, status;
    int fd[2];
    pid_t pid;
 
    success = 0;
    total = argc - 1;
    for (int i = 1; i < argc; i++) {
        pipe(fd);
        pid = fork();
        if (pid < 0) {
            perror("Fork fallita.");
            kill(0, SIGKILL);
        } else if (pid == 0) {
            close(fd[1]);
            dup2(fd[0], 0);
            close(fd[0]);
            execl("./cracker", "cracker", NULL);
            perror("Exec fallita.");
            exit(EXIT_FAILURE);
        } else {
            close(fd[0]);
            write(fd[1], argv[i], strlen(argv[i])+1);
            close(fd[1]);
        }
    }
 
    while (wait(&status) >= 0) {
        success += WIFEXITED(status) ? 1 - WEXITSTATUS(status) : 0;
    }
    printf("Crackati %d hash su %d!\n", success, total);
 
    return EXIT_SUCCESS;
}