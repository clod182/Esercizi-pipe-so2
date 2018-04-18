#include <semaphore.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>

#define MAX 100
#define TEST_THREADS 1000
#define GREETINGS "Hello, I'm producing this!"
#define MAXBUF 100
#define DELAY 10000

sem_t mymymutex;
int failed=0;

/**************** DA CONSEGNARE *****************/

/*
 * Lo scopo della verifica è la realizzazione di un buffer circolare in cui:
 * - letture/scritture simultanee da parte di più thread non devono interferire tra loro;
 * - l'operazione di lettura è bloccante: se nel buffer non sono presenti abbastanza dati,
 *   il consumatore legge i byte disponibili e rimane in attesa dei rimanenti;
 * - analogamente, la scrittura è bloccante: se nel buffer non sono presenti abbastanza celle
 *   vuote, il produttore riempie quelle disponibili e attende che si liberi spazio.
 *
 * La sincronizzazione è realizzata utilizzando i seguenti semafori:
 * - 'mutex_lettori' viene utilizzato per serializzare le operazioni di lettura da parte di più thread.
 *   Il semaforo è inizializzato a 1, in quanto la lettura dev'essere eseguita da un thread alla volta.
 * - 'mutex_scrittori' viene utilizzato per serializzare le operazioni di scrittura da parte di più thread.
 *   Anche questo semaforo è inizializzato a 1 per permettere la scrittura da parte di un thread alla volta.
 * - 'celle_piene' conta il numero di celle piene all'interno del buffer ed è utilizzato per implementare
 *   il comportamento bloccante della lettura: all'inizio il buffer è vuoto, dunque è inizializzato a 0.
 * - 'celle_vuote' conta il numero di celle vuote all'interno del buffer ed è utilizzato per implementare
 *   il comportamento bloccante della scrittura: all'inizio il buffer è vuoto, dunque è inizializzato a MAX.
 * 
 * Alcune verifiche consegnate usano anche il semaforo 'mutex', come mostrato nei commenti della soluzione
 * proposta, per evitare interferenze nel caso in cui un lettore e un consumatore operino sulla stessa cella del
 * buffer. È comunque corretto, nel dubbio è meglio sincronizzare un po' di più rispetto a sincronizzare meno ed
 * avere interferenze indesiderate. Nel nostro caso, tuttavia, non era necessario usarlo osservando che:
 * - se inserisci != preleva, allora lettore e scrittore stanno operando su celle del buffer diverse;
 * - se inserisci == preleva, allora il buffer è pieno oppure è vuoto, dunque un thread tra lettore e scrittore
 *   è bloccato nell'attesa di una cella vuota/piena.
 */

//  Coda, da sincronizzare
typedef struct coda {
	char buffer[MAX]; 	// buffer circolare di dimensione MAX
	int inserisci; 		// indice per l'inserimento
	int preleva;
    sem_t mutex_lettori;
    sem_t mutex_scrittori;
    sem_t celle_piene;
    sem_t celle_vuote;   		// indice per il prelievo 
	// Aggiungere qui i semafori 
} coda;

// Inizializza la coda e semafori
void cInit(coda *c) {
	c->inserisci=0;
	c->preleva=0;
    sem_init(&c->mutex_lettori, 0, 1);
    sem_init(&c->mutex_scrittori, 0, 1);
    sem_init(&c->celle_piene, 0, 0);
    sem_init(&c->celle_vuote, 0, MAX);
}

// Distrugge i semafori
void cClose(coda *c) {
    sem_destroy(&c->mutex_lettori);
    sem_destroy(&c->mutex_scrittori);
    sem_destroy(&c->celle_piene);
    sem_destroy(&c->celle_vuote);
}

/*
   cRead:
   Legge n byte dalla coda c e li scrive in s. Se non ci sono abbastanza byte da leggere, legge fino a svuotare la coda e poi si blocca in attesa dei byte mancanti.
   Letture simultanee da piÃ¹ thread non devono interferire: se due thread eseguono cRead, uno dei due attende che l'altro abbia terminato la propria lettura.
*/
void cRead(coda *c, char *s, int n) {
    int i;
 
    /* Accedi in lettura al buffer in mutua esclusione: attendi se c'è già
     * un thread che sta leggendo. */
    sem_wait(&c->mutex_lettori);

    for (i = 0; i < n; i++) {
        /* Attendi che ci sia almeno una cella piena nel buffer della coda. */
        sem_wait(&c->celle_piene);
        // sem_wait(&c->mutex);
        /* Inserisci il carattere letto nel buffer s e aggiorna la posizione da
         * cui leggere il carattere successivo. */
        s[i] = c->buffer[c->preleva];
        c->preleva = (c->preleva + 1) % MAX;
        // sem_post(&c->mutex);
        /* Ora la cella da cui è stato letto il carattere può essere sovrascritta. */
        sem_post(&c->celle_vuote);
    }
     
    /* Esci dalla sezione critica e permetti ad un altro lettore di procedere. */
    sem_post(&c->mutex_lettori);
}

/*
   cWrite:
   Scrive i primi n byte di s nella coda c. Se la coda si riempie attende che si liberi spazio.
   Scritture simultanee effettuate da piÃ¹ thread non devono interferire. Se due thread eseguono cWrite uno dei due attende che l'altro abbia terminato la propria scrittura.
*/
void cWrite(coda *c, char *s, int n) {
    
    int i;
 
    /* Accedi in scrittura al buffer in mutua esclusione: attendi se c'è già
     * un thread che sta scrivendo. */
    sem_wait(&c->mutex_scrittori);
    for (i = 0; i < n; i++) {
        /* Attendi che ci sia almeno una cella vuota nel buffer della coda. */
        sem_wait(&c->celle_vuote);
        // sem_wait(&c->mutex);
        /* Inserisci il carattere letto nel buffer della coda e aggiorna la posizione
         * in cui scrivere il carattere successivo. */
        c->buffer[c->inserisci] = s[i];
        c->inserisci = (c->inserisci + 1) % MAX;
        // sem_post(&c->mutex);
        /* Ora la cella in cui è stato scritto il carattere può essere letta. */
        sem_post(&c->celle_piene);
    }
 
    /* Esci dalla sezione critica e permetti ad un altro scrittore di procedere. */
    sem_post(&c->mutex_scrittori);
}

/*************** FINE PARTE DA CONSEGNARE *********************/


void die(char *s, ...) {
	va_list args;
	va_start(args,s);

	sem_wait(&mymymutex);
	vprintf(s,args);
	va_end(args);
	failed=1;
	sem_post(&mymymutex);
	pthread_exit(NULL);
}

void *produttore(void *p) {
	coda *c = (coda *)p;
	cWrite(c,GREETINGS,strlen(GREETINGS));

}

void *consumatore(void *p) {
	coda *c = (coda *)p;
	char buf[MAXBUF];
	if (MAXBUF < strlen(GREETINGS)+1) {
		die("overflow!\n");
	}
	usleep(DELAY*2);

	cRead(c,buf,strlen(GREETINGS));
	buf[strlen(GREETINGS)] = '\0';

	if (strcmp(buf,GREETINGS) != 0) {
		die("[consumatore] Ho letto:\n%s\ninvece di:\n%s!\n",buf,GREETINGS);
	}
}

int main() {
	int i;
	coda c;
	pthread_t tidp[TEST_THREADS];
	pthread_t tidc[TEST_THREADS];

	sem_init(&mymymutex,0,1);

	cInit(&c);
	for (i=0;i<TEST_THREADS;i++) {
		pthread_create(&tidp[i],NULL,produttore,&c);
	}
	for (i=0;i<TEST_THREADS;i++) {
		pthread_create(&tidc[i],NULL,consumatore,&c);
	}

	for (i=0;i<TEST_THREADS;i++) {
		pthread_join(tidp[i],NULL);
		pthread_join(tidc[i],NULL);
	}
	cClose(&c);

	if (failed) {
		printf("Errore di sincronizzazione!\n");
	} else {
		printf("Well done! Adesso commentalo bene! :)\n");
	}
}