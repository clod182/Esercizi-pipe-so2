/*---------------------versione con dimensione cifra acquisita libera ossia senza vincoli di numero caratteri che la rappresentano------------------*/
 
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
 
/*Si deve realizzare una semplice calcolatrice in grado di sommare una lista di numeri, secondo le seguenti specifiche:
 
1-  La calcolatrice è in un ciclo infinito: continua a prendere input e dare output finché non viene interrotta prende l’input da una pipe ‘calcPipeIn’ 
        e manda i risultati su una seconda pipe ‘calcPipeOut’. 
         
2-      Le pipe vengono create dalla calcolatrice stessa quando viene eseguita le espressioni sono 
        semplici somme, ad esempio 10 + 15 + 280. 
         
3-  Vengono inviate su calcPipeIn come sequenze di char terminate da #. Ad esempio 10 + 15 + 280 viene inviata 
        come ’10+15+280#’, ovvero 10 byte senza il terminatore di stringa e senza spazi 
         
4-  I risultati vengono inviati sempre come sequenze di char separati 
        da ‘#’. Se, ad esempio, inviamo ’1+2#3+4#’ su calcPipeIn ci aspetteremo ’3#7#’ su calcPipeOut
*/
 
#define NUM "calcPipeIn"    /*pipe di ingresso per la scrittura dove io scriverò il mio risultato tramite write*/
#define RIS "calcPipeOut"   /*pipe dedicato al recupero della stringa che avrà valore di espressione numerica*/
 
int main(){
    int fd[2];  /*array che gestisce per convenzione in posizione '0'--> LETTURA e in posizione '1'--> SCRITTURA delle Pipe*/
    int somma=0;    /*inizializzo la variabile che conterrà il risultato in forma di 'INTERI'*/
    char risultato[5];/*variabile che userò per inviare il risultato alla mia pipe in forma di 'stringa'*/
    int cifra;  /*variabile di servizio che conterrà il di volta in volta la mia cifra convertita da stringa in intero*/
    int i=0;
     
/*----variabili di servizio---------*/
     
    int d=10;
    char n;     /*mi è sufficiente un solo carattere per l'aqcuisizione della cifra sottoforma di stringa, poichè lo converto
            in numero decimale di volta in volta, così da non avere 'limiti' sulla dimensione della stringa acquisita*/
    int parz=0;
     
/*----------------------------------*/
     
    /*1° devo aprire un canale di comunicazione tra i due processi generati da quello originario, sfruttando la sistem call 'open'*/
    /*array che gestisce per convenzione in posizione '0'--> LETTURA e in posizione '1'--> SCRITTURA delle Pipe*/
    fd[0] = open(NUM, O_RDONLY);    /*apro la 'pipe' in lettura*/
    if(fd[0]<0){         
        perror("\nErrore nell'appertura del file in lettura");
        exit(1);        /*se la syscall 'open' genera un errore in scrittura della 'pipe' lo devo gestrite terminando il programma*/
    }
    /*NB: NON E' NECESSARIO, ANZI E' ERRATO CONSIDERARE L'OPZIONE "ELSE" IN QUANTO SE SONO ENTRATO NELL'IF IL PROCESSO TERMINA*/
    /*a questo punto l'apertura del canale di comunicazione in lettura è andato a buon fine, posso provare ad eseguire l'apertura
    del canale di scrittura*/
    fd[1] = open(RIS, O_WRONLY);    /*apro la 'pipe' in scrittura*/
    if(fd[1]<0){
        perror("\nErrore nell'appertura del file in scrittura");
        exit(1);        /*se la syscall 'open' genera un errore in scrittura della 'pipe' lo devo gestrite terminando il programma*/
    }
 
/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    while(read(fd[0],&n,1)){    /*entro in un ciclo dove inizio l'acquisizione dei miei numeri finchè non incontro il '#'*/
            if(n=='+'){ /*ho trovato una cifra, quindi la devo sommare alle precedenti*/
                somma=somma+parz;   /*ho acquisito il simbolo '+' e quindi devo fare la somma e resettare la somma parziale*/
                parz=0;
            }
            else    if(n=='#'){
                somma=somma+parz;   /*è arrivato il carattere di fine stringa e quindi dovrò ricostruire il mio intero in stringa*/
                parz=0;         /*resetto la somma parziale*/
                sprintf(risultato,"%d#", somma);        /*converto il mio risultato numerico in stringa*/
                /*lo invio alla pipe in uscita con i seguenti parametri: cod. porta(fd[1]), stringa, dimensione*/
                write (fd[1],risultato,strlen(risultato));  
                    somma=0;        /*resetto il risultato della somma effettuata per ricominciare con una nuova iterazione*/
                }
                else{
                    cifra=n-'0';    /*converto il carattere acquisito in un intero*/
                    parz=parz*10;   /*se sono qui significa che il carattere acquisito non è un '+' o '#' quindi alle somme parziali devo 
                            continuamente 'aggiungere' il 'decimo' della cifra acquisita*/
                    parz=parz+cifra;/*proseguo poi con la mia somma parziale, ad es: 123 +
                                        parz=0
                            acquisisco: 1   parz=parz*10 = 0    parz=parz+1 = 1
                            acquisisco: 2   parz=parz*10 = 10   parz=parz+2 = 12
                            acquisisco: 3   parz=parz*10 = 120  parz=parz+3 = 123   
                            ecco perchè ho bisogno di un solo carattere  per l'acquisizione del mio dato 'stringa'*/
                             
                }
    }
     
    close(fd[0]);   /*chiudo la mia pipe sia in lettura che in scrittura*/
    close(fd[1]);
    return 0;   
}