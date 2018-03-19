#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define PIPEIN "carPipeIn"
#define PIPEOUT "carPipeOut"
#define BUFSIZE 15
#define TURNIDAFARE 20

int main(){
    int i, fd_in, fd_out;
    int holepos, carpos, turni=0, mosse;
    char carattere, overflow, buffer[BUFSIZE];

    mkfifo(PIPEOUT, 0666);
    mkfifo(PIPEIN, 0666);
    fd_in = open(PIPEOUT, O_RDONLY);
    if (fd_in < 0) {
        fprintf(stderr, "Non riesco ad aprire '%s' in lettura.\n", PIPEOUT);
        exit(EXIT_FAILURE);
    }
    fd_out = open(PIPEIN, O_WRONLY);
    if (fd_out < 0) {
        fprintf(stderr, "Non riesco ad aprire '%s' in scrittura.\n", PIPEIN);
        exit(EXIT_FAILURE);
    }   
    
    while(turni < TURNIDAFARE) {
        i=0;
        while(i < BUFSIZE && read(fd_in, buffer + i , 1) && buffer[i] != '#'){
            i++;
        }
        sscanf(buffer, "%d#", &holepos);        
        mosse = abs(carpos - holepos);
        
        //aggiorno ultima posizione        
        //incremento contatore
    }
    close(fd_in);
    close(fd_out);
    return EXIT_SUCCESS;
}