#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <semaphore.h>
#include <string.h>
 
sem_t sem;
 
/*
gcc esercizio2.c -lpthread -o test
 
Creo il semaforo (inizializzato a 0) e i due thread. Il primo thread esegue la funzione T1, una volta eseguito il punto A aumenta il valore del semaforo.
Il secondo thread esegue la funzione T2, si blocca se il semaforo è rosso (cioè se deve ancora essere eseguito il punto A).
Attendo la terminazione sei thread e elimino il semaforo.
*/
 
void die(char * s, int e) {
    printf("%s [%i]\n",s,e);
    exit(1);
}
 
// primo thread
void * T1(void * j) {
	sleep(3);
	printf("Eseguito A\n");
    sem_post(&sem);
	sleep(3);
	printf("Eseguito B\n");
    pthread_exit(NULL);
}
 
// secondo thread
void * T2(void * j) {
	printf("Eseguito C\n");
    sem_wait(&sem); // semaforo bloccante nel caso non sia ancora stato eseguito il punto A
	printf("Eseguito D\n");
    pthread_exit(NULL);
}
 
int main(){
  int i,err;
  pthread_t tid[2];
 
  //creo il semaforo e lo inizializzo a 0
  sem_init(&sem,0,0);
 
  //creo i due thread
  if (err=pthread_create(&tid[0],NULL,T1,NULL))
      die("errore create",err);
 
  if (err=pthread_create(&tid[1],NULL,T2,NULL))
      die("errore create",err);
 
  // attende i thread.
  for (i=0;i<2;i++)
      if (err=pthread_join(tid[i],NULL))
          die("errore join",err);
 
  sem_destroy(&sem);
 
}