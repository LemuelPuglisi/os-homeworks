#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

/**
 * Shell minimale che sfrutta l'exec per 
 * mandare in esecuzione i programmi
 * 
 * @todo: introdurre il passaggio degli argomenti 
 */

#define BUFFER_SIZE 2048

int main (int argc, char* argv[])
{
    char command[BUFFER_SIZE]; 
    int cmdlen, pid; 


    printf("Welcome to mint shell (Write \"quit\" for exit)\n"); 

    while (1) {

        // prendo il comando dall'utente
        printf("[user] "); 
        fgets(command, BUFFER_SIZE, stdin); 

        cmdlen = strlen(command); 

        if (command[cmdlen - 1] == '\n') {
            command[cmdlen - 1] = '\0'; 
        }

        if (strcmp(command, "quit") == 0) {
            break; 
        }

        if ((pid = fork()) == -1) {
            printf("[error] shell fork error\n"); 
        } 

        if (pid == 0) {
            execlp(command, command, NULL);

            // se il programma prosegue allora vuol dire
            // che la chiamata exec ha automaticamente 
            // fallito, poiché dopo di essa il processo
            // dovrebbe swappare programma. 
            printf("[error] command %s not found.\n", command); 
            exit(1);  
        }
        else { wait(NULL); }
    }

    printf("Goodbye!\n");
    exit(0);  
}