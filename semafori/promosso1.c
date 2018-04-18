#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include "promosso2.c"
#define DELAY 10000
#define SCHEDULE DELAY*100
#define X 80
#define Y 26

sem_t mutex_stampa;
sem_t mutex_attivi;
volatile int n_attivi=0;
volatile int n_mossi=0;

/************** PARTE DA CONSEGNARE **************/

/**
 * Scopo della verifica è sincronizzare i thread pixel che compongono la scritta iniziale "BOCCIATO!!" in modo che
 * questa si trasformi in "PROMOSSO :)". I thread pixel sono attivati da un thread scheduler in modo che compongano
 * una lettera/carattere alla volta. È inoltre necessario rispettare i seguenti vincoli:
 * - i pixel che compongono una lettera devono muoversi contemporaneamente (in caso contrario, infatti, scatta un
 *   timeout che termina il programma);
 * - due o più pixel non possono occupare contemporaneamente la medesima casella.
 * 
 * Per realizzare quanto richiesto, utilizziamo:
 * - una matrice di semafori ('campo') per determinare quali posizioni del campo sono libere o meno: in particolare,
 *   il semaforo in posizione (x, y) sarà rosso se la posizione è già occupata da un pixel, verde altrimenti;
 * - un vettore di semafori ('turno') usato dallo scheduler per comunicare ai pixel quando sono autorizzati ad
 *   iniziare lo spostamento;
 * - un vettore di semafori ('mosso') usato dai pixel per comunicare allo scheduler che hanno terminato il loro
 *   spostamento.
 */
 
sem_t campo[X][Y];
sem_t turno[SCRITTALEN];
sem_t mosso[SCRITTALEN];

// definire qui i semafori

void inizializza() {
	int i, j;
 
    /* Inizialmente le posizioni del campo sono tutte libere, quindi tutti i semafori sono inizializzati a 1. */
    for (i = 0; i < X; i++)
        for (j = 0; j < Y; j++)
            sem_init(&campo[i][j], 0, 1);
    /* Lo scheduler non ha ancora autorizzato alcun pixel a muoversi, quindi i semafori del vettore 'turno' sono
     * tutti inizializzati a 0. Inoltre nessun pixel ha chiaramente raggiunto la posizione finale, quindi anche
     * i semafori del vettore 'mosso' sono a 0. */
    for (i = 0; i < SCRITTALEN; i++) {
        sem_init(turno + i, 0, 0);
        sem_init(mosso + i, 0, 0);
    }
}

void distruggi() {
	int i, j;
 
    /* Distruggi i vari semafori. */
    for (i = 0; i < X; i++)
        for (j = 0; j < Y; j++)
            sem_destroy(&campo[i][j]);
    for (i = 0; i < SCRITTALEN; i++) {
        sem_destroy(turno + i);
        sem_destroy(mosso + i);
    }
}

void inizia_mossa(int x, int y) {
	/* Accedi alla cella in posizione (x, y) in mutua esclusione: se è già occupata, attendi. */
    sem_wait(&campo[x][y]);
}

void fine_mossa(int x, int y) {
	/* Il pixel sta per abbandonare la casella: notifica che la posizione è libera. */
    sem_post(&campo[x][y]);
}

void attendi_lettera(int i) {
	/* Lo scheduler attende che il pixel con id 'i' abbia completato lo spostamento nella posizione finale. */
    sem_wait(mosso + i);
}

void fatta_lettera(int i) {
	/* Il pixel 'i' notifica al thread scheduler di essersi spostato con successo nella posizione finale. */
    sem_post(mosso + i);
}

void attendi_turno(int i) {
	/* Il pixel 'i' attende che lo scheduler autorizzi l'inizio dello spostamento. */
    sem_wait(turno + i);
}

void dai_turno(int i) {
	/* Lo scheduler autorizza il pixel 'i' ad iniziare a muoversi. */
    sem_post(turno + i);
}

/*********** FINE PARTE DA CONSEGNARE ************/

char board[X][Y];

void init_board() {
	int i,j;

	for (i=0;i<X;i++)
		for (j=0;j<Y;j++)
			board[i][j] = ' ';

	for (i=0;i<SCRITTALEN;i++) {
		board[bocciato[i].x][bocciato[i].y] = '#';
	}
}

void stampa() {
	int i,j;

	printf("\e[1;1H\e[2J"); // clear
	fflush(stdout);
	for (j=0;j<Y;j++) {
		for (i=0;i<X;i++) {
			putchar(board[i][j]);
		}
		putchar('\n');
	}
	fflush(stdout);
}

// muore in modo thread safe ...
void die(char * s) {
	sem_wait(&mutex_stampa);
    printf("\n--> %s\n",s);
    fflush(stdout);
    exit(1);
}

void *pixel_code(void *i) {
	int id=*(int *)i;
	int nx,ny,x,y,targetx,targety;
	int randomized=0;

	x=bocciato[id].x;
	y=bocciato[id].y;

	targetx=promosso[id].x;
	targety=promosso[id].y;

	attendi_turno(id); // da implementare

	/************** conta gli attivi per il test ***************/
	sem_wait(&mutex_attivi);
	n_attivi++;
	sem_post(&mutex_attivi);
	/***********************************************************/

	while( (x != targetx) || (y != targety) ) {

		/**** algoritmo smart per evitare i deadlock :) *****/
		if (randomized>0) {
			nx = (x + random() % 3 - 1) % X;
			ny = (y + random() % 3 - 1) % Y;
			if (nx < 0) nx=0;
			if (ny < 0) ny=0;
			randomized--;
		} else {
			nx = x - (x > targetx) + (x < targetx);
			ny = y - (y > targety) + (y < targety);
		}
		/****************************************************/

		if (nx >= X || nx < 0 || ny >= Y || ny < 0)
			die("Overflow 1");

		inizia_mossa(nx,ny); // da implementare!

		while (board[nx][ny] != ' ') {
			fine_mossa(nx,ny);
			nx = (x + random() % 3 - 1) % X;
			ny = (y + random() % 3 - 1) % Y;
			if (nx < 0) nx=0;
			if (ny < 0) ny=0;

			if (nx >= X || nx < 0 || ny >= Y || ny < 0)
				die("Overflow 2");
			if (randomized==0)
				randomized=2;
			inizia_mossa(nx,ny);
		} 
		usleep(DELAY);
		if (board[nx][ny] != ' ') {
			die("Sto sovrascrivendo!");
		}

		/**** muove il pixel e conta le mosse per il test ***/
		sem_wait(&mutex_stampa);
		board[x][y] = ' ';
		board[nx][ny] = '#';
		sem_wait(&mutex_attivi);
		n_mossi++;
		sem_post(&mutex_attivi);		
		sem_post(&mutex_stampa);
		/****************************************************/

		x = nx;
		y = ny;

		fine_mossa(nx,ny); // da implementare!

	}
	sem_wait(&mutex_attivi);
	n_attivi--; // per il test non toccare!
	sem_post(&mutex_attivi);

	fatta_lettera(id);

	pthread_exit(0);
}

void *scheduler_code(void *w) {
	int i,j;
	int turno=0;

	for(i=0;i<10;i++) {

		/* dà il turno alla lettera */
		for(j=0;j<SCRITTALEN;j++) {
			if ((promosso[j].x / 8) == turno)
				dai_turno(j); // da implementare!
		}

		/* attende che la lettera sia completa */
		for(j=0;j<SCRITTALEN;j++) {
			if ((promosso[j].x / 8) == turno){
				attendi_lettera(j); // da implementare
				if (board[promosso[j].x][promosso[j].y] == ' ')
					die("Non hai completato la lettera!");
			}
		}
		turno++;
	}
	pthread_exit(0);
}

/* printer non fa parte dell'esercizio non toccare */
void *printer_code(void *j) {
	int i;
	int mosse_totali=0;
	int attivi_totali=0;
	while(1) {
		usleep(DELAY);

		sem_wait(&mutex_stampa); 
		stampa();
		sem_wait(&mutex_attivi);

		attivi_totali+=n_attivi;
		mosse_totali+=n_mossi;
		n_mossi=0;
		sem_post(&mutex_attivi);
		sem_post(&mutex_stampa);

		// test molto smart!
		if (n_attivi > 100 && mosse_totali > 100) {
				die("Hei! una lettera alla volta!");
		}
		if (attivi_totali - mosse_totali > 400) {
				die("Troppo lenti!");
		}
	}
	pthread_exit(0);
}


int main() {
	int i,ret,tid[SCRITTALEN];
	pthread_t pixel[SCRITTALEN];
    pthread_t scheduler,printer;

    srandom(time(NULL)); // initialize random seed

    /* semafori del test NON TOCCARE! */
	sem_init(&mutex_stampa,0,1);
	sem_init(&mutex_attivi,0,1);
	/**********************************/

	scritta_init(); // inizializza le coordinate
	init_board(); // inizializza la board
	stampa();

	inizializza(); // inizializza semafori (da implementare)

	// crea i thread pixel
	for (i=0;i<SCRITTALEN;i++) {
		tid[i]=i;
        if((ret=pthread_create(&pixel[i],NULL,pixel_code,&tid[i])))
            die("errore create");
	}

	// crea lo scheduler
    if ((ret=pthread_create(&scheduler,NULL,scheduler_code,NULL)))
        die("errore create");

    // crea il printer
    if ((ret=pthread_create(&printer,NULL,printer_code,NULL)))
        die("errore create");

    // attende pixel e scheduler
    for (i=1;i<SCRITTALEN;i++)
        if((ret=pthread_join(pixel[i], NULL)))
             die("errore join");
    pthread_join(scheduler,NULL);

    distruggi(); // elimina i semafori (da implementare)

    usleep(DELAY*2); // attende che la stampa venga eseguita
}