/* Seconda verifica di Lab Sistemi Operativi (a.a. 2011-2012)
   Ricordarsi di commentare il codice e di spiegare, brevemente, la soluzione proposta
 */
 

//NOTA: stallo tra filosofi risolto ricorrendo al filosofo mancino, vedere le funzioni in fondo a questo file per i dettagli.


sem_t mutex; 					//mutex per la sezione critica, che consta nel verificare numero pasti.
sem_t bacchette[N_FILOSOFI];	//array di semafori per le bacchette.
sem_t lettura;			   //variabile BOOL che indica se un filosofo sta "leggendo" o meno dalla tavola, per evitare che più filosofi prendano lo stesso piatto.

//categorico -> pasti va inizializzato a 0.
int pasti = 0;			   //all'inizio non ci sono pasti in tavola. (Se ne mettessimo 3 i filosofi prenderebbero lo stesso piatto e non va bene.)

void init_sem() {
	sem_init(&mutex,0,1);
	sem_init(&lettura,0,1);
	int i = 0;
	for(i = 0; i< N_FILOSOFI; i++){
		sem_init(&bacchette[i],0,1);		//inizializzo le bacchette a 1 perchè sono sulla tavola e sono disponibili.
	}
}

void destroy_sem() {
	sem_destroy(&mutex);
	sem_destroy(&lettura);
	int i = 0;
	for(i = 0; i< N_FILOSOFI; i++){
		sem_destroy(&bacchette[i]);
	}
}

//filosofo semwait per CONSUMARE pasti. Attende che ce ne siano.
void ini_leggi() {
	sem_wait(&lettura);				//aspetta se c'è gia un altro filosofo che sta prendendo il piatto x.
	int flag = 0;
	while(!flag){					//cicla in loop chiedendo e rilasciando il semaforo mutex (così il cameriere può servire)
		sem_wait(&mutex);
		if(pasti > 0){				//se c'è un pasto disponibile esci dal ciclo, e nel main il programma proseguirà.
			flag = 1;
		}
		sem_post(&mutex);
	}
}

///filosofo sem_post per notificare che HA CONSUMATO il pasto
void end_leggi() {
	sem_wait(&mutex);
	pasti--;					//aggiorna la variabile pasti in sezione critica.
	sem_post(&mutex);

	sem_post(&lettura);			//io, [FILOSONFO N], mi sono servito. Lascio spazio al filosofo successivo.
}

//cameriere che aspetta di poter/dover consegnare il pasto.
void ini_scrivi() {
	int flag = 0;
	while(!flag){
		sem_wait(&mutex);
		if(pasti<3){		//se c'è un posto per inserire un nuovo pasto esci dal ciclo, e nel main il cameriere proseguirà.
			flag = 1;
		}
		sem_post(&mutex);
	}
}

//cameriere che notifica di avere consegnato il pasto.
void end_scrivi() {
	sem_wait(&mutex);
	pasti++;					//aggiorna la variabile pasti in sezione critica.
	sem_post(&mutex);
}
 
void raccogli_sx(int b) {
	if(b == 0){	//filosofo mancino
		sem_wait(&bacchette[(b + 1) % N_FILOSOFI]);
	}
	else	//gli altri 4 filosofi
		sem_wait(&bacchette[b]);
}

void raccogli_dx(int b) {
	if(b == 0){	//filosofo mancino
		sem_wait(&bacchette[b]);
	}
	else	//gli altri 4 filosofi
		sem_wait(&bacchette[(b + 1) % N_FILOSOFI]);
}

//qui non distinguiamo tra filosofo mancino o destro.
void deposita_sx(int b) {
	sem_post(&bacchette[b]);
}

void deposita_dx(int b) {
	sem_post(&bacchette[(b+1)%N_FILOSOFI]);
}