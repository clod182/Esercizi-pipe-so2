#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdarg.h>
#include "filosofi.c"
#define N_COPERTI 15

// funzioni di terminazione
void die(char * s, int i) {
   printf("[ERROR] %s: %i\n",s,i);
   exit(1);
}
void die2(char *s, ...) {
   va_list ap;

   va_start(ap, s);
   vprintf(s,ap);
   va_end(ap);

   exit(1);
}

// Variabili per il test, non toccare
int a_tavola_test[N_FILOSOFI];
int n_coperti=N_COPERTI;
sem_t mymymy_mutex; // usato per la terminazione, non toccare!

// modulo che funziona anche per numeri negativi
int my_mod(int n) {
   n = n%N_FILOSOFI;
   if (n<0) {
       n+=N_FILOSOFI;
   }
   return n;
}

// funzioni di test non toccare!
void test_mangia(int id) {
   sem_wait(&mymymy_mutex);
   if (a_tavola_test[my_mod(id-1)] || a_tavola_test[my_mod(id+1)]) {
       die2("[Filosofo %d] Sto mangiando assieme a un vicino!\n",id);
   }
   a_tavola_test[id]=1;
   sem_post(&mymymy_mutex);
}
void fine_test_mangia(int id) {
  a_tavola_test[id]=0;
}

// decrementa il contatore e decide se uscire, non toccare!
void test_exit() {
   sem_wait(&mymymy_mutex);
   if (n_coperti == 0) {
       sem_post(&mymymy_mutex);
       pthread_exit(0);
   }
   n_coperti--;
   sem_post(&mymymy_mutex);
}

void test_cameriere(int id) {
  sem_wait(&mymymy_mutex);
  if (a_tavola_test[id]) {
    die2("[Cameriere %d] Sto preparando quando il filosofo sta ancora mangiando!\n");
  }
  sem_post(&mymymy_mutex);
}

void * cameriere(void * n) {
   int id = * (int *) n;

   while(1) {
       prepara_coperto(id); // prepara il coperto in posizione id
       test_cameriere(id);
       printf("[Cameriere %d] Sistemato il coperto\n",id);

       test_exit(); // decide se uscire, non toccare
   }
}

void * filosofo(void * n) {
   int id = * (int *) n;

   while(1) {

       printf("[Filosofo %d] Che fame, mangio qualcosa ...\n",id);

       raccogli_sx(id); // raccoglie la bacchetta sinistra
       printf("[Filosofo %d] Ho raccolto la bacchetta sinistra\n",id);

       sleep(1); // forza il deadlock

       raccogli_dx(id); // raccoglie la bacchetta destra
          printf("[Filosofo %d] Ho raccolto la bacchetta destra... mangio!\n",id);

          // mangia e controlla la mutua esclusione
          test_mangia(id);
          sleep(3);
          fine_test_mangia(id);

          printf("[Filosofo %d] Finito, torno a pensare!\n",id);
          alzati(id); // Si alza da tavola portandosi via le bacchette  

          sleep(3);
   }
}

int main() {
   pthread_t th1[N_FILOSOFI], th2[N_FILOSOFI];
   int th1_id[N_FILOSOFI];
   int i,ret;

   sem_init(&mymymy_mutex,0,1); // non toccare!

   // inizializza i semafori
   init_sem(); 

   for (i=0;i<N_FILOSOFI;i++)
       a_tavola_test[i]=0; // per il test

   // crea i filosofi
   for (i=0;i<N_FILOSOFI;i++) {
       th1_id[i]=i;
       if((ret=pthread_create(&th1[i],NULL,filosofo,&th1_id[i])))
           die("errore create",ret);
       printf("Creato il filosofo %i\n", th1_id[i]);
   }

   // fa partire i camerieri, un po' dopo per verificare la sincronizzazione
   sleep(2);

   // crea i camerieri   
      for (i=0;i<N_FILOSOFI;i++) {
       if((ret=pthread_create(&th2[i],NULL,cameriere,&th1_id[i] )))
           die("errore create",ret);
       printf("Creato il cameriere\n");
   }

   // attende la terminazione dei camerieri
   for (i=0;i<N_FILOSOFI;i++)
       if((ret=pthread_join(th2[i], NULL)))
           die("errore join",ret);

   // elimina i semafori
   destroy_sem();
   printf("Terminato correttamente\n");
   
   return 0;
}