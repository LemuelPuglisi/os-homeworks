/*---------------------------------------------------------------------------------------*/
/* L'esercizio non è completo. È stata implementata solo la funzione LIST causa tempo.   */
/*---------------------------------------------------------------------------------------*/ 

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>

#include "strsplit.c"

#define IPC_KEY_GEN_FILE "key/secure_ipc_key"
#define IPC_KEY_GEN_COMM 32 // coda dei comandi
#define IPC_KEY_GEN_DATA 64 // coda dei dati

#define TESTING_PROCESS_NUMBER 5  
#define COMMAND_BUFFER_SIZE 1024

#define DISPLAY_PROCESS_TYPE 10

// La struct utilizzata all'interno della coda di messaggi
// contiene un primo elemento "type" che specifica
// quale degli n processi debba servire la richiesta, ed 
// un ulteriore elemento per il testo del messaggio. 

typedef struct {
    long type; 
    char text[1024]; 
} message; 

// inserisce un messaggio nella coda e suddivide il contenuto in 
// più chunks se la dimensione è maggiore di 1024 bytes. 

void enqueue(short queue_desc, char * data, short type) 
{
    message msg; 
    msg.type = type; 
    size_t payload_size = strlen(data); 

    short cut_times = 0, 
             offset = 0;  

    while (payload_size > 1024) {

        offset = cut_times * 1024;
        payload_size -= 1024; 
        bzero(msg.text, 1024); 
        memcpy(msg.text,  offset + data, 1024); 

        msgsnd(queue_desc, &msg, 1024, 0);
    } 

    memcpy(msg.text, offset + data, 1024); 
    msgsnd(queue_desc, &msg, 1024, 0);
}

// un handler prende in ingresso un intero "type". Tale intero 
// si aggira nell'intorno [1, n], dove n è il numero di cartelle
// in input. Esso gestisce la i-esima cartella. 

void handler (short type, char * dirname, short command_queue_desc, short data_queue_desc)
{
    short bytes_received; 
    message message_buffer; 
    DIR * directory = opendir(dirname); 

    if (!directory) exit(0); 

    while (1) {
    
        bytes_received = msgrcv(command_queue_desc, &message_buffer, 1024, type, 0); 
        
        if (bytes_received == -1) {
            perror("[msgrcv]"); 
            exit(1); 
        }

        if (strcmp(message_buffer.text, "quit") == 0) 
            exit(0); 

        if (strcmp(message_buffer.text, "list") == 0) {
            
            // conto quanti messaggi verranno inviati 
            struct dirent * dir_element;
            // struct stat * file_info; 
            while ((dir_element = readdir(directory)) != NULL ) {
                if (dir_element->d_type == DT_REG) {
                    
                    short name_len = strlen(dir_element->d_name); 
                    char message_with_endline[name_len + 1];
                    sprintf(message_with_endline, "%s\n", dir_element->d_name); 
                    
                    enqueue(data_queue_desc, message_with_endline, DISPLAY_PROCESS_TYPE);  
                } 
            }
        }   

    }

    exit(0); 
}

void message_displayer (short message_queue_desc) 
{
    message msg; 
    short bytes_received; 

    while (1) {
        bytes_received = msgrcv(message_queue_desc, &msg, 1024, DISPLAY_PROCESS_TYPE, 0); 
        msg.text[bytes_received - 1] = '\0'; 
        printf("%s", msg.text);
    }

    exit(0); 
}

// il processo principale fornisce una shell per interfacciarsi con
// l'utente e comunica i comandi ai processi figli (handlers). 
// Tali comandi vengono inseriti nella coda dei comandi. 

int main(int argc, char * argv[])
{
    int pid; 
    char user_command_buffer[COMMAND_BUFFER_SIZE]; 

    short   directories = argc - 1, 
            handlers = directories; 

    // generiamo due chiavi per due code distinte. 
    key_t command_queue_key = ftok(IPC_KEY_GEN_FILE, IPC_KEY_GEN_COMM);
    key_t data_queue_key    = ftok(IPC_KEY_GEN_FILE, IPC_KEY_GEN_DATA);  

    // creo la coda dei comandi e la coda dei dati. 
    // Nella prima transiteranno i comandi dal processo principale
    // ai processi handler, nella seconda transiteranno i dati 
    // nel verso opposto. 
    short command_queue_descriptor = msgget(command_queue_key, IPC_CREAT | 0660); 
    short data_queue_descriptor = msgget(data_queue_key, IPC_CREAT | 0660);

    if (command_queue_descriptor < 0|| data_queue_descriptor < 0) {
        perror("[msgget]"); 
        exit(1); 
    }

    // avviamo n processi 
    for (int i = 1; i <= handlers; ++i) 
        if ((pid = fork()) == 0) {
            handler(i, argv[i], command_queue_descriptor, data_queue_descriptor); 
            perror("[fork (handlers)]"); 
            exit(0); 
        }

    // avviamo il processo che si occupa del display dei messaggi
    // e ne conserviamo il PID per terminarlo in chiusura. 
    short display_process_pid = fork(); 
    if (display_process_pid == 0) {
        message_displayer(data_queue_descriptor); 
        perror("[fork (display)]"); 
        exit(0); 
    }

    // avviamo la shell utente 
    bzero(user_command_buffer, COMMAND_BUFFER_SIZE);

    // output degli handler 
    printf("-----------------------------------------------------\n"); 
    for (int i = 1; i <= handlers; i++) 
        printf("| [\033[0;32mactive handler\033[0m] n.%d | directory: %s \n", i, argv[i]); 
    if (handlers == 0) 
        printf("| \033[0;31mno active handlers.\033[0m \n"); 
    printf("-----------------------------------------------------\n"); 
    printf("available commands: list <handler>, quit \n\n"); 


    while (strcmp(user_command_buffer, "quit") != 0) {
        
        bzero(user_command_buffer, COMMAND_BUFFER_SIZE); 
        fgets(user_command_buffer, COMMAND_BUFFER_SIZE, stdin);

        short command_length = strlen(user_command_buffer); 
        if (command_length > 1 && user_command_buffer[command_length - 1] != '\0')
            user_command_buffer[command_length - 1] = '\0';  

        char ** command_words = strsplit(user_command_buffer, ' '); 

        if (strcmp(command_words[0], "quit") == 0) 
            break; 

        short list_command_referenced = strcmp(command_words[0], "list") == 0; 
        short size_command_referenced = strcmp(command_words[0], "size") == 0; 
        short srch_command_referenced = strcmp(command_words[0], "search") == 0; 

        if (list_command_referenced || size_command_referenced || srch_command_referenced) {
            
            short handler_selected = command_words[1] != NULL
                ? atoi(command_words[1])
                : -1;

            if (handler_selected <= 0 || handler_selected > handlers) {
                printf("> [error] Specify a valid handler number after command.\n"); 
            }
            else {
                enqueue(command_queue_descriptor, command_words[0], handler_selected);
                // command output 
            }
        }
        else printf("> [error] Command not found.\n");
    }

    // let all the child processes exit passing a quit command 
    for (int i = 1; i <= handlers; ++i) 
        enqueue(command_queue_descriptor, "quit", i);

    kill(display_process_pid, SIGTERM); 
    msgctl(command_queue_descriptor, IPC_RMID, NULL); 
    msgctl(data_queue_descriptor, IPC_RMID, NULL); 
}