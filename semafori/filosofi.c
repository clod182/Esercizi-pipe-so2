/*

I filosofi igienisti
====================
(Verifica per il corso di Sistemi Operativi aa 2016-17, Ca' Foscari, Venezia)

Ci sono N_FILOSOFI al ristorante che, come al solito, pensano e mangiano a una tavola
circolare, condividendo le bacchette con i vicini.

Inizialmente ci sono N_FILOSOFI bacchette e i filosofi prelevano la bacchetta sinistra
e poi quella destra per mangiare.

Quando un filosofo finisce di mangiare butta via le proprie bacchette e un cameriere le
sostituisce con bacchette pulite, in modo che due filosofi non mangino mai con la stessa
bacchetta usata.

Lo schema del filosofo è:

    while(1) {
        raccogli_sx(id); // raccoglie la bacchetta sinistra
        raccogli_dx(id); // raccoglie la bacchetta destra

        // mangia

        alzati(id); // Si alza da tavola portandosi via le bacchette  

        // pensa
    }

Lo schema del cameriere (uno per filosofo) è:

    while(1) {
        prepara_coperto(id); // prepara il coperto in posizione id
    }

Il cameriere deve attendere che il filosofo si alzi prima di sostituire il coperto quindi la
chiamata a prepara_coperto(id) deve sincronizzarsi con alzati(id).

Implementare la funzioni di sincronizzazione in modo che filosofi e camerieri si coordino
correttamente ed evitando lo stallo tra i filosofi.

Questo file è incluso da main.c. Il main include già il file semaphore.h, non serve aggiungere
l'include anche qui. Per compilare è dunque sufficiente il seguente comando:

    gcc main.c -o main -lpthread
*/

/*
 * La verifica consiste nell'implementazione di una variante del classico
 * problema dei filosofi a cena. In questa variante sono presenti tanti
 * camerieri quanti sono i filosofi: dopo aver finito di mangiare, un
 * filosofo non deposita le bacchette sul tavolo (come nella versione
 * classica), ma è il cameriere associato al filosofo che deve occuparsi
 * di posizionare sul tavolo delle bacchette nuove.
 *
 * È dunque necessario realizzare:
 * - la sincronizzazione tra il filosofo e il corrispondente cameriere per
 *   la disposizione delle bacchette sulla tavola quando il primo ha finito
 *   di mangiare;
 * - la sincronizzazione tra i filosofi nella raccolta delle bacchette,
 *   impedendo il verificarsi di situazioni di stallo.
 * 
 * Per la gestione delle bacchette utilizziamo il vettore di semafori chiamato
 * 'bacchette': il semaforo i-esimo vale 1 se la bacchetta i-esima è depositata
 * sul tavolo, 0 altrimenti.
 *
 * Per la sincronizzazione tra camerieri e filosofi utilizziamo il vettore di
 * semafori 'cambio_coperto': il semaforo i-esimo vale 1 se il filosofo si è
 * alzato ed il cameriere deve posizionare le nuove bacchette, 0 altrimenti.
 *
 * Per la risoluzione del problema dello stallo è possibile adottare varie
 * tecniche spiegate a lezione, quali il filosofo mancino o la limitazione
 * del numero dei posti a tavola. È preferibile evitare la raccolta atomica
 * delle bacchette poiché "sincronizza troppo" i filosofi, come spiegato
 * nella lezione 'Programmazione con i semafori'.
 * Nella soluzione proposta limitiamo il numero di filosofi che possono
 * contemporaneamente competere nella raccolta delle bacchette a 'N_FILOSOFI-1':
 * dal momento che le bacchette disponibili sono 'N_FILOSOFI', almeno uno
 * riuscirà a raccogliere entrambe le bacchette e mangiare. Il semaforo 'posti'
 * viene utilizzato proprio a questo scopo.
 */
 
#define N_FILOSOFI 5
 
sem_t cambio_coperto[N_FILOSOFI];
sem_t bacchette[N_FILOSOFI];
sem_t posti;
 
/* Inizializzazione dei semafori. */
void init_sem() {
  /* Come descritto sopra, limitiamo il numero di posti a tavola a
   * 'N_FILOSOFI-1'. */
  sem_init(&posti, 0, N_FILOSOFI-1);
  for (int i = 0; i < N_FILOSOFI; i++) {
    /* Inizialmente le bacchette sono già posizionate sul tavolo, dunque i
     * camerieri devono rimanere in attesa. */
    sem_init(cambio_coperto+i, 0, 0);
    sem_init(bacchette+i, 0, 1);
  }
}
 
/* Rimozione dei semafori. */
void destroy_sem() {
  sem_destroy(&posti);
  for (int i = 0; i < N_FILOSOFI; i++) {
    sem_destroy(cambio_coperto+i);
    sem_destroy(bacchette+i);
  }
}
 
void prepara_coperto(int id) {
  /* Attendi che il filosofo si alzi dal tavolo. */
  sem_wait(cambio_coperto+id);
  /* Riposiziona sul tavolo le bacchette usate dal filosofo. */
  sem_post(bacchette+id);
  sem_post(bacchette + (id+1) % N_FILOSOFI);
}
 
void alzati(int id) {
  /* Notifica al cameriere che deve preparare il coperto. */
  sem_post(cambio_coperto+id);
  /* Libera il posto a tavola per altri filosofi. */
  sem_post(&posti);
}
 
void raccogli_sx(int id) {
  /* Attendi che si liberi un posto a tavola. */
  sem_wait(&posti);
  /* Raccogli la bacchetta sinistra: attendi se è già in uso dal filosofo
   * seduto a sinistra. */
  sem_wait(bacchette+id);
}
 
void raccogli_dx(int id) {
  /* Raccogli la bacchetta destra: attendi se è già in uso dal filosofo
   * seduto a destra. */
  sem_wait(bacchette + (id+1) % N_FILOSOFI);
}