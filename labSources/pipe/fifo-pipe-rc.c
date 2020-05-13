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
 * [RECEIVER]
 * Questo programma legge dati da una fifo (named pipe)
 * aperta nella cartella /tmp.   
 * 
 */

#define BUFFER_SIZE 2048

int main (int argc, char *argv[]) {

    const char * pathname = "/tmp/fifo";
    char buffer[BUFFER_SIZE]; 

    // apro la fifo
    int pipefd = open(pathname, O_RDONLY); 

    if (pipefd < 0) {
        perror(pathname); 
        exit(1); 
    }

    for (int i = 0; i < 10; i++) {

        read(pipefd, buffer, BUFFER_SIZE); 
        printf("messaggio ricevuto: %s\n", buffer); 

    }

    exit(0); 
}