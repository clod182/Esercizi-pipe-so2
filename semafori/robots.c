#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

#define N_ROBOTS 10
#define X 20
#define Y 80

char board[X][Y];
struct info_r {
    int id;
    int x;
    int y;
} info_r;

/******* PARTE DA CONSEGNARE ********/
sem_t robots[N_ROBOTS];
sem_t giocatore[N_ROBOTS];
sem_t mutex;
int *distrutto;/*indica se il robot i è stato distrutto oppure no*/
/* inizializza semafori e variabili */
void inizializza() {
    int i;
    for(i=0;i<N_ROBOTS;i++){
        sem_init(&robots[i], 0, 1);/*li inizzializzo a 1 e non a 0 per non creare un attesa circolare*/
        sem_init(&giocatore[i], 0, 0);
    }

    sem_init(&mutex, 0, 1);

    distrutto = (int*) calloc(N_ROBOTS, sizeof(int));/*inizzializzo l'array distrutto (calloc inizializza a 0 le celle allocate a differenza di malloc)*/
}

/* viene invocata prima di ogni mossa */
void inizia_mossa() {
    sem_wait(&mutex);
}

/* viene invocata dopo ogni mossa */
void fine_mossa() {
    sem_post(&mutex);
}

/* il robot id attende che il giocatore si muova */
void robot_attendi_mossa(int id) {
    sem_wait(&giocatore[id]); /*attende che il giocatori li comunichi che si è mosso*/
}

/* il robot id si è mosso, il giocatore può fare la mossa successiva */
void robot_mossa_effettuata(int id) {
    sem_post(&robots[id]);
}

/* il giocatore attende che i robot si muovano tutti */
void player_attendi_mossa() {
    int i;

    for(i=0;i<N_ROBOTS;i++)
        if(!distrutto[i])/*se il robot è stato distrutto non lo aspetta*/
            sem_wait(&robots[i]);/*aspetta che tutti i robot abbiano effettuato la loro mossa*/

}

/* il giocatore si è mosso, possono muoversi i robots */
void player_mossa_effettuata() {
    int i;
    for(i=0;i<N_ROBOTS;i++)
        sem_post(&giocatore[i]);/*il robot i puo ora muoversi*/
}

/* elimina i semafori */
void chiudi() {
    int i;
    for(i=0;i<N_ROBOTS;i++){
        sem_destroy(&robots[i]);/*elimino i semafori*/
        sem_destroy(&giocatore[i]);
    }
    sem_destroy(&mutex);
}
void robot_termina(id){/*il robot id è stato distrutto*/
    distrutto[id]=1;
}
/******** FINE PARTE DA CONSEGNARE ********/

void die(char * s, int i) {
    printf("--> %s numero %i\n",s,i);
    exit(1);
}

/* leggere un carattere da terminale */
int getch ( void ) {
  int ch;
  struct termios origterm, tmpterm;

  tcgetattr ( STDIN_FILENO, &origterm );
  tmpterm = origterm;
  tmpterm.c_lflag &= ~( ICANON | ECHO );
  tcsetattr ( STDIN_FILENO, TCSANOW, &tmpterm );
  ch = getchar();
  tcsetattr ( STDIN_FILENO, TCSANOW, &origterm );

  return ch;
  }

/*
 * muove p da x,y a nx,ny
 * - se in x,y non c'e' significa che p e' stato schiacciato (ritorna 0)
 * - se in nx,ny c'e' qualcuno lo stiamo schiacciando e scriviamo '#' (ritorna 0)
 * - se si sposta correttamente ritorna 1
 */
muovi(int x,int y,int nx,int ny,char p) {
    if (board[x][y] != p) {
        return 0; // schiacciato!
    } else {
        board[x][y] = ' ';
        if (board[nx][ny] != ' ') {
            board[nx][ny] = '#';
            return 0; // schiaccia
        } else {
            board[nx][ny] = p;
            return 1; // OK
        }
    }
}

void *robot_code(void *i) {
    int px,py,dx,dy,found,mossa;
    struct info_r info = *(struct info_r *)i;
    int id = info.id;
    int x=info.x,y=info.y;

    while (1) {
        // attende che il giocatore si sia mosso
        robot_attendi_mossa(id);

        /***************** computa la mossa ********************/
            // cerca il giocatore su tutta la board
            found=0;
            for (px=0;px<X && !found;px++)
                for (py=0;py<Y && !found;py++)
                    if (board[px][ py] == '@') {
                        found=1;
                    }
            px--;py--;

            // calcola la mossa
            dx = (px>x) - (px<x);
            dy = (py>y) - (py<y);
        /*******************************************************/

        inizia_mossa();

        /****************** esegue la mossa ********************/
            mossa = muovi(x,y,x+dx,y+dy,'+');
        /*******************************************************/

        fine_mossa();

        /******** a seconda dell'esito inibisce il robot ******/
            if (!mossa) { // fallimento
                // un robot quando muote continua a sincronizzarsi per sempre!
#ifdef EXTRA
                robot_termina(id);
                robot_mossa_effettuata(id);
                pthread_exit(0);
#else
                while(1) {
                    robot_mossa_effettuata(id);
                    robot_attendi_mossa(id);
                }
#endif
            }
            x = x+dx; y = y+dy;
        /******************************************************/

        robot_mossa_effettuata(id);

    }
}

void *player_code(void *i) {
    char c;
    struct info_r infop = *(struct info_r *)i;
    int x=infop.x,y=infop.y,dx,dy,nx,ny,mossa;

    while(1) {
        player_attendi_mossa();

        /***** stampa il board e legge il comando ************/
            print_board();

            c = getch();
            dx=0;dy=0;
            switch (c) {
                case 'e':
                    dx=-1;dy=-1;break;
                case 'r':
                    dx=-1;dy=0;break;
                case 't':
                    dx=-1;dy=1;break;
                case 'd':
                    dx=0;dy=-1;break;
                case 'f':
                    // teletransport
                    dx = (int) ( (float)random()*X/RAND_MAX ) - x;
                    dy = (int) ( (float)random()*Y/RAND_MAX ) - y;
                    break;
                case 'g':
                    dx=0;dy=1;break;
                case 'c':
                    dx=1;dy=-1;break;
                case 'v':
                    dx=1;dy=0;break;
                case 'b':
                    dx=1;dy=1;break;
            }
            nx=x+dx; ny=y+dy;
        /*******************************************************/

        inizia_mossa();

        /****************** esegue la mossa ********************/
            mossa = muovi(x,y,x+dx,y+dy,'@');
        /*******************************************************/

        fine_mossa();

        /****** a seconda dell'esito termina o prosegue  *******/
            if (!mossa) {
                printf("player is dead\n");
                exit(0); // player is dead
            }
            x=x+dx;y=y+dy;
        /*******************************************************/

        player_mossa_effettuata();

    }
}

print_board() {
    int x,y;
    printf("+");
    for (y=0;y<Y;y++)
        printf("-");
    printf("+\n");

    for (x=0;x<X;x++) {
        printf("|");
        for (y=0;y<Y;y++)
            printf("%c",board[x][y]);
        printf("|\n");
    }

    printf("+");
    for (y=0;y<Y;y++)
        printf("-");
    printf("+\n");
}

int posiziona(int x,int y,char c) {
    if (board[x][y] == ' ') {
        board[x][y] = c;
        return 1; // libera, prendi il posto
    } else
        return 0; // gia' occupata
}

main() {
    pthread_t robots[N_ROBOTS];
    pthread_t you;
    int x,y,i,ret;
    struct info_r info[N_ROBOTS];
    struct info_r infop;

    srandom(time(NULL)); // initialize random seed

    // reset board
    for (x=0;x<X;x++)
        for (y=0;y<Y;y++)
            board[x][y] = ' ';

    // inizializza le strutture dati e i semafori
    inizializza();

    // crea i thread
    for (i=0;i<N_ROBOTS;i++) {
        info[i].id = i;
    // sceglie una nuova posizione libera
        do {
            info[i].x = (int) ( (float)random()*X/RAND_MAX );
            info[i].y = (int) ( (float)random()*Y/RAND_MAX );
        } while (!posiziona(info[i].x,info[i].y,'+'));

        if(ret=pthread_create(&robots[i],NULL,robot_code,(void *)&info[i]))
            die("errore create",ret);
    }

    printf("Creati %i robots \n", N_ROBOTS);

    do {
        infop.x = (int) ( (float)random()*X/RAND_MAX );
        infop.y = (int) ( (float)random()*Y/RAND_MAX );
    } while (!posiziona(infop.x,infop.y,'@'));

    if (ret=pthread_create(&you,NULL,player_code,(void *)&infop))
        die("errore create",ret);

    printf("Creato il giocatore \n");

    // attende la terminazione dei thread
    for (i=0;i<N_ROBOTS;i++)
        if(ret=pthread_join(robots[i], NULL))
             die("errore join",ret);
    printf("Bravo hai vinto!\n");

    // distrugge i semafori
    chiudi();

}