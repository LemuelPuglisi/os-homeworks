#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/**
 * [TRANSMITTER]
 * Questo programma trasmette dati attraverso una fifo
 * (named pipe) aperta nella cartella /tmp. Si occupa
 * solo della scrittura dei dati, non leggerà mai.  
 * 
 */

int main (int argc, char *argv[]) {

    const char * pathname = "/tmp/fifo"; 
    const char * message = "hello!"; 

    // se la named pipe esiste già, 
    // la elimino
    unlink(pathname); 

    // creo la named pipe nella cartella 
    // specificata assegnando permessi 
    // rwx rispetto all'utente corrente
    int response = mkfifo(pathname, 0777);

    if (response == -1) {
        perror(pathname); 
        exit(1);  
    } 

    // apro la fifo appena creata
    int pipefd = open(pathname, O_WRONLY); 

    // se l'apertura da problemi, 
    // mi assicuro di eliminare la
    // fifo creata
    if (pipefd < 0) {
        perror(pathname); 
        unlink(pathname); 
        exit(1); 
    }

    for (int i = 0; i < 10; i++) {
        // scriviamo il messaggio nella fifo 
        response = write(pipefd, message, strlen(message) + 1); 
        printf("messaggio %d inviato\n", i); 

        // Se il ricevente non è a conoscenza
        // della lunghezza del messaggio inviato, 
        // allora potrebbe accorpare più messaggi
        // distinti in uno solo e provocare anomalie. 
        sleep(1); 
    }

    close(pipefd); 
    unlink(pathname); 
}