#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <semaphore.h>
#include <string.h>
 
 
/*
gcc esercizio1.c -lpthread -o test -O3
 
Inizializzo il semaforo a 1, creo i due thread che incrementano la variabile passatgli come argomento solo nel caso in cui il semaforo sia verde.
Attendo la terminazione dei thread.
*/
 
sem_t sem;
 
// stampa gli errori ed esce
void die(char * s, int e) {
    printf("%s [%i]\n",s,e);
    exit(1);
}
 
// il thread incrementa 100000 volte la variabile passata come argomento
void * codice_thread(void * a) {
    int *b = (int*)a;
    int i;
    for(i=0;i<100000;i++){
      sem_wait(&sem);
      (*b)++;
      sem_post(&sem);
    }
    pthread_exit(NULL);
}
 
int main() {
 
    pthread_t tid[2];
    int i,err;
    int count=0;
 
    //creazione semaforo
    sem_init(&sem,0,1);
 
 
    // crea i thread. Viene passato come argomento l'indirizzo della variabile intera count (che viene incrementata dai thread)
    for (i=0;i<2;i++)
        if (err=pthread_create(&tid[i],NULL,codice_thread,&count))
            die("errore create",err);
 
    // attende la terminazione dei due thread
    for (i=0;i<2;i++)
        if (err=pthread_join(tid[i],NULL))
            die("errore join",err);
 
    printf("I thread hanno terminato l'esecuzione correttamente e count è %d\n",count);
 
    //distruzione semaforo
    sem_destroy(&sem);
 
}