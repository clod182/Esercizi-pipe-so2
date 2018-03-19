#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>//time(NULL) per il seed dei numeri casuali di rand()

#define PIPELETTURA "lettura"
#define PIPESCRITTURA "scrittura"
#define BSIZE 50
int main(int argc, char *argv[]) {
  int flettura, fscrittura;
  int giorno, mese, anno;
  char buffer[BSIZE];
  char compleanno[] = "11/02/1996";
  char doposbornia[50];
  //apertura pipe in lettura e scrittura
  flettura = open(PIPELETTURA, O_RDONLY);
  fscrittura = open(PIPESCRITTURA, O_WRONLY);

  //resettare il seed del generatore di numeri casuali
  srand(time(NULL));

  //intero casuale
  rand();

  memset(buffer, 0, BSIZE); //setta a 0 i primi BSIZE byte di buffer

  //scrivere in una pipe
  write(fscrittura, "Stringa", strlen("Stringa"));

  //leggere da una pipe
  read(flettura, buffer, 1); //legge 1 byte dalla pipe ritorna 0 sse pipe vuota e chiusa in scrittura

  //sscanf
  sscanf(compleanno, "%d/%d/%d", &giorno, &mese, &anno);
  giorno++;
  memset(doposbornia, 0, 50);
  sprintf(doposbornia, "%d/%d/%d", giorno, mese, anno);
  printf("%s\n", doposbornia);

  return 0;
}
