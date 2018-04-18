#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <semaphore.h>
#include <string.h>
 
#define N_PR 5 //numero produttori
#define N_CO 5 //numero consumatori
#define MAX 100 //dimensione buffer
#define N_CICLI 150 //numero iterazioni
 
/*
gcc esercizio3.c -lpthread -o test
 
Inizializzo tre semafori (piene = 0, vuote = MAX  e mutex = 1) e un buffer di dimensione MAX con valore speciale -1, dove -1 indica la cella vuota.
Creo N_CO consumatori e N_PR produttori che vanno a leggere e scrivere sul buffer.
Il produttore richiede che almeno una cella sia libera. Se sta per scrivere su una cella non libera (!= -1) stampa un messaggio d'errore e termina l'intero processo.
Il consumatore richiede che almeno una cella sia stata scritta da un produttore. Se sta per leggere da una cella libera (== -1) stampa un messaggio d'errore e
termina l'intero processo.
Attendo la terminazione di tutti i thread ed elimino i semafori.
*/
 
sem_t piene, vuote, mutex;
 
int preleva = 0,inserisci = 0;
int buffer[MAX];
 
// stampa gli errori ed esce
void die(char * s, int e) {
    printf("%s [%i]\n",s,e);
    exit(1);
}
 
void* Produttore(void *j) {
  int i=0;
  while(i<N_CICLI) { //ogni thread compie N_CICLI iterazioni
    sem_wait(&vuote); // richiede una cella vuota
    sem_wait(&mutex); // aspetta il proprio turno
    if(buffer[inserisci] != -1)
      die("ERRORE: Scrittura in cella non vuota",inserisci); //se sta per scrivere in una cella non vuota termina l'intero processo
 
    buffer[inserisci] = rand()%99;
    printf("Prodotto valore --> %d\n",buffer[inserisci]);
    inserisci = (inserisci+1) % MAX;
 
    sem_post(&mutex); // rilascia il turno
    sem_post(&piene); // rilascia una cella piena
    i++;
  }
  pthread_exit(NULL);
}
 
void* Consumatore(void *j) {
  int i=0;
  while(i<N_CICLI) {
    sem_wait(&piene); // richiede una cella piena
    sem_wait(&mutex); // aspetta il proprio turno
    if(buffer[preleva] == -1)
      die("ERRORE: Lettura in cella vuota",preleva); //se sta per leggere una cella vuota termina l'intero processo
 
    printf("Letto valore %d\n",buffer[preleva]);
    buffer[preleva] = -1;
    preleva = (preleva+1) % MAX;
 
    sem_post(&mutex); // rilascia il turno
    sem_post(&vuote); // rilascia una cella vuota
    i++;
  }
  pthread_exit(NULL);
}
 
int main(){
 
  int i,err;
  pthread_t produttori[N_PR],consumatori[N_CO];
 
  // iniziallizzo semafori
  sem_init(&piene,0,0);
  sem_init(&vuote,0,MAX);
  sem_init(&mutex,0,1);
 
  // all'inizio tutte le celle sono vuote (=-1)
  for(i=0;i<MAX;i++)
    buffer[i] = -1;
 
  //creazione produttori
  for(i=0;i<N_PR;i++)
      if (err=pthread_create(&produttori[i],NULL,Produttore,NULL))
        die("errore create",err);
 
 
 
  //creazione consumatori
  for(i=0;i<N_CO;i++)
      if (err=pthread_create(&consumatori[i],NULL,Consumatore,NULL))
        die("errore create",err);
 
  //attesa produttori
  for (i=0;i<N_PR;i++)
      if (err=pthread_join(produttori[i],NULL))
          die("errore join",err);
  //attesa consumatori
  for (i=0;i<N_CO;i++)
     if (err=pthread_join(consumatori[i],NULL))
          die("errore join",err);
 
  //eliminazione semafori
  sem_destroy(&piene);
  sem_destroy(&vuote);
  sem_destroy(&mutex);
 
}