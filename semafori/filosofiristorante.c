#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<semaphore.h>
#include<unistd.h>
#include<stdarg.h>
 
#define N_PASTI 15
#define DIM_BUFFER 3
#define N_FILOSOFI 5
 
#include "filosofiris.c"   // funzioni di sincronizzazione DA REALIZZARE PER LA VERIFICA
 
/********** le funzioni qui sotto sono 'ACCESSORIE' al test, non serve guardarle in dettaglio *********/
 
// funzioni di terminazione
void die(char * s, int i) {
    printf("[ERROR] %s: %i\n",s,i);
    exit(1);
}
/*void die2(char * s) {
    printf("[SYNC ERROR] %s\n",s);
    exit(1);
}*/
     
void die2(char *s, ...) {
    va_list ap;
 
    //printf("[SYNC ERROR] ",s);
 
    va_start(ap, s);
    vprintf(s,ap);
    va_end(ap); 
 
    exit(1);
 
}
// buffer circolare
struct {
    int buf[DIM_BUFFER];
    int inserisci;
    int preleva;
} buffer;
 
// scrive i nel buffer
void scrivi_buffer(int i) {
    buffer.buf[buffer.inserisci]=i;
    buffer.inserisci=(buffer.inserisci+1)%DIM_BUFFER;
}
 
// legge un intero dal buffer id
int leggi_buffer() {
    int j=buffer.buf[buffer.preleva];
    #ifdef CHECK_MUTEX
    sleep(1);
    #endif
    buffer.preleva=(buffer.preleva+1)%DIM_BUFFER;
    return j;
}
 
int bacchette_test[N_FILOSOFI]; // le bacchette, utilizzate per il test
int pasti_test[N_PASTI]; // conteggia i pasti per il test
int pasti_consumati=0; // tutti i pasti sono stati consumati
 
// consuma il pasto e controlla che le bacchette siano utilizzate correttamente
void consuma_pasto(int id, int i) {
    int j;
    int id_dx = (id+1)%N_FILOSOFI;
 
    if (bacchette_test[id]) die2("[Filosofo %i] Bacchetta %d gia' in uso\n",id,i);
    if (bacchette_test[id_dx]) die2("[Filosofo %i] Bacchetta %d gia' in uso\n",id_dx,i);
 
    bacchette_test[id] = bacchette_test[id_dx] = 1;
 
    printf("[Filosofo %i] Consumo il pasto %i\n",id,i);
    sleep(2);
 
    bacchette_test[id] = bacchette_test[id_dx] = 0;
 
    if (pasti_test[i]) {
        die2("[ERRORE] sto per consumare il pasto %i gia' consumato in precedenza\n",i);
    }
    pasti_test[i]=1; // pasto consumato
    for (j=0;j<N_PASTI && pasti_test[j];j++);
    if (j==N_PASTI)
        pasti_consumati=1; // e' ora di uscire 
}
 
void * cameriere(void * n) {
    int i;
         
    for (i=0;i<N_PASTI;i++) {
        printf("[Cameriere] Consegno il pasto %i\n",i);
         
        ini_scrivi();
         
          scrivi_buffer(i); // scrive i nel buffer
 
        end_scrivi();
    }
}
 
void * filosofo(void * n) {
    int id = * (int *) n;
    int i;
 
    while(1) {
        ini_leggi();
         
            i=leggi_buffer(); // prende il pasto dal buffer
 
         
        end_leggi();
         
        printf("[Filosofo %d] Ho ricevuto il pasto %d\n",id, i);
 
        raccogli_sx(id);
            sleep(1); // forza il deadlock
        raccogli_dx(id);
 
            consuma_pasto(id,i); // consuma il pasto
 
        deposita_sx(id);
        deposita_dx(id);       
    } 
}
 
int main() {
    pthread_t th1[N_FILOSOFI], th2;
    int th1_id[N_FILOSOFI];
    int i,ret;
 
    // inizializza i semafori
    init_sem();  
 
    for (i=0;i<N_PASTI;i++)
        pasti_test[i]=0; // per il test
 
    // crea i filosofi
    for (i=0;i<N_FILOSOFI;i++) {
        th1_id[i]=i;
        if((ret=pthread_create(&th1[i],NULL,filosofo,&th1_id[i])))
            die("errore create",ret);
        printf("Creato il filosofo %i\n", th1_id[i]);
    }
 
    // fa partire il cameriere un po' dopo per verificare la sincronizzazione
    sleep(2); 
    // crea il cameriere    
    if((ret=pthread_create(&th2,NULL,cameriere,NULL ))) 
        die("errore create",ret);
    printf("Creato il cameriere\n");
             
    /* attende la terminazione
    for (i=0;i<N_FILOSOFI;i++)
        if((ret=pthread_join(th1[i], NULL)))
            die("errore join",ret);
            */
    if((ret=pthread_join(th2, NULL)))
            die("errore join",ret);
 
    for (i=0;i<5 && !pasti_consumati;i++) {
        printf("[MAIN] Attendo che i pasti siano consumati\n");
        sleep(10);
    }
 
    // elimina i semafori
    destroy_sem();
    if (i==5)
        die2("I pasti non sono stati tutti consumati\n");
    else {
    printf("Terminato correttamente\n");
        exit(0);
    }
}