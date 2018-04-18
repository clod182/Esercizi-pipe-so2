#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <semaphore.h>
#include <stdint.h>
 
#define N 100   // Numero di viaggiatori
#define CIN 10  // Numero postazioni di check-int
 
/*********** variabili per il testing ************/
    int check_postazione[CIN];
    sem_t check_mutuo;
/*************************************************/
 
void * viaggiatore(void * i);
 
void die(char * s, int i) {
    printf("--> %s numero %i\n",s,i);
    exit(1);
    }
 
 
/*********** parte da consegnare ****************/
 
// dichiarazione di variabili globali
int i=0;
sem_t sempostazioni[N];
sem_t mutex;
 
// funzioni del main
void inizializza() {
    for(i = 0; i < N; i++) {
        sem_init(&sempostazioni[i],0,0); 
    }
    sem_init(&mutex,0,1);
}
 
 
void chiudi() {
    for(i = 0; i < N; i++) {
        sem_destroy(&sempostazioni[i]);
    } 
    sem_destroy(&mutex);
}
 
// funzioni del viaggiatore
int attendi_postazione() {  
    int i;  
    //sem_wait(&mutex);
    for (i = 0; i < N; i++)
        sem_wait(&sempostazioni[i]);
    //sem_post(&mutex);
    return i;
}
 
void libera_postazione(int j) {
    sem_post(&sempostazioni[j]);
}
/*********fine parte da consegnare **************/
 
 
void * viaggiatore(void * i) {
    int j;
    intptr_t id = (intptr_t)i;
 
    printf("[Viaggiatore %d] Entro in aeroporto\n", (int) id);
 
    j = attendi_postazione();
     
    /************** testing ***************/
        sem_wait(&check_mutuo);
        if (check_postazione[j]==1) 
            die("postazione gia' occupata",j);
        check_postazione[j]=1;
        sem_post(&check_mutuo);
    /**************************************/
     
    printf("[Viaggiatore %d] Sto usando lo sportello %d \n", (int) id, j);
     
    /*************** testing **************/
        sleep(1);
        check_postazione[j]=-1;
    /**************************************/
 
    libera_postazione(j);
         
     
    printf("[Viaggiatore %d] Vado al gate! \n", (int) id);
                     
}
 
crea_thread(pthread_t *th) {
    intptr_t i;
    int ret;
 
    for (i=0;i<N;i++)
        if(ret=pthread_create(&th[i],NULL,viaggiatore,(void *) i)) 
            die("errore create",ret);
}
 
attendi_thread(pthread_t *th) {
    intptr_t i;
    int ret;
 
    for (i=0;i<N;i++)
        if(ret=pthread_join(th[i], NULL))
             die("errore join",ret);
}
 
 
main() {
    pthread_t th[N];
    int i;
 
    sem_init(&check_mutuo,0,1);
         
    for (i=0; i<CIN; i++)
        check_postazione[i] = 0;
         
    // inizializza le strutture dati e i semafori
    inizializza();
 
    // crea i thread viaggiatori    
    crea_thread(th);
     
    printf("Creati %i viaggiatori \n", N);
 
    // attende la terminazione dei thread viaggiatori
    attendi_thread(th); 
     
    // distrugge i semafori 
    chiudi();
     
    /********************* testing *******************/
        for (i=0; i<CIN; i++) {
            if (check_postazione[i] == 0)
                die("postazione mai utilizzata",i);
        }
    /*************************************************/
}