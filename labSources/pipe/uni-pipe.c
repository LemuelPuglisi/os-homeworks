#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <limits.h>

int main (int argc, char *argv[]) {

    int pid, cmdr; 
    int pipefd[2]; 
    char buffer[PIPE_BUF]; 

    // apro una pipe assegnando all'elemento
    // 0 il canale di lettura della pipe ed 
    // all'elemento 1 il canale di scrittura
    // della pipe 
    cmdr = pipe(pipefd);
    
    // faccio una fork del processo 
    // corrente per far comunicare
    // processo padre con processo 
    // figlio
    pid  = fork(); 


    if (pid == 0) {
 
        // il processo figlio leggerà dal
        // canale di lettura (0) della pipe
        // e non scriverà nulla. Di conseguenza
        // è buona norma chiudere l'altro canale 
        close(pipefd[1]); 
        read(pipefd[0], buffer, PIPE_BUF);
        
        printf("[son] dad said '%s' to me\n", buffer); 
        exit(0); 
    }
    else {

        printf("[dad] sending a message to my son through pipe channel.\n");  
        
        const char * message = "hello son!"; 

        // analogamente, il padre chiude il
        // canale di lettura della pipe
        // e scrive sul canale di scrittura
        // il messaggio da invare al processo
        // figlio. Ne aspetta la risposta per 
        // concludere il processo. 
        close(pipefd[0]); 
        write(pipefd[1], message, strlen(message)); 
        
        wait(NULL);

        printf("[dad] son opened my message.\n");  
    }

    exit(0); 
}