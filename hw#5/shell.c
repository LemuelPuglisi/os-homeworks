#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "strsplit.c"

#define TRUE 1 
#define COMMAND_BUFFER_LENGTH 4096
#define STDIN   0
#define STDOUT  1
#define STDERR  2

int check_for_quit_command(char * command) 
{
    return strcmp(command, "quit") == 0; 
}

/*
    Homework n.5

    Estendere l'esempio 'nanoshell.c' ad una shell piu' realistica in cui
    si possono:
    - passare argomenti al comando invocato (per semplicita', assumiamo
    che questi non contengano spazi);
    - usare la redirezione dei canali di input/output/error;
    - mandare un comando in esecuzione in background (con la '&' finale).

    Esempi di invocazione che la shell dovrebbe supportare sono:
    $ cal 3 2015
    $ cp /etc/passwd /etc/hosts /tmp
    $ cat </dev/passwd >/tmp/passwd
    $ cat filechenonesiste 2>/dev/null
    $ ls -R /etc/ &
*/


int main (int argc, char * argv[])
{
    int command_length, 
        pid; 

    char command_buffer[COMMAND_BUFFER_LENGTH]; 
    bzero(command_buffer, COMMAND_BUFFER_LENGTH); 

    char * command; 
    char ** arguments; 

    char * file_to_replace_stdin  = NULL; 
    char * file_to_replace_stdout = NULL; 
    char * file_to_replace_stderr = NULL; 

    int background_execution = 0; 

    for(;;) {

        file_to_replace_stderr =
            file_to_replace_stdout = 
            file_to_replace_stdin  = NULL; 

        // if (background_execution)
        //     fflush(stdout);

        background_execution = 0; 
        printf("[shell] >> "); 
        fgets(command_buffer, COMMAND_BUFFER_LENGTH, stdin);

        command_length = strlen(command_buffer); 
        if (command_buffer[command_length - 1] != '\0')
            command_buffer[command_length - 1] = '\0'; 

        if (check_for_quit_command(command_buffer) == TRUE) 
            break; 

        int total_arguments_passed = count_word_between_delimiter(command_buffer, ' '); 
        char ** arguments = strsplit(command_buffer, ' '); 

        // inspect for some stream redirection and count valid
        // arguments, assigning NULL pointer to non-valid arguments 
        int valid_arguments = 0;
        for (int i = 0; i < total_arguments_passed; i++) { 
            if (arguments[i] != NULL) { 

                if (strlen(arguments[i]) == 1 && arguments[i][0] == '&') {
                    arguments[i] = NULL; 
                    background_execution = 1; 
                }
                else if (strlen(arguments[i]) == 1 && arguments[i][0] == '>') {
                    if (arguments[i+1] != NULL) {
                        file_to_replace_stdout = strdup(arguments[i+1]); 
                        // both the > operator and the output stream 
                        // destination aren't valid arguments. So we 
                        // can set them to null. 
                        arguments[i] = NULL;
                        arguments[i+1] = NULL;  
                    }
                }
                else if (strlen(arguments[i]) == 1 && arguments[i][0] == '<') {
                    if (arguments[i+1] != NULL) {
                        file_to_replace_stdin = strdup(arguments[i+1]); 
                        arguments[i] = NULL;
                        arguments[i+1] = NULL;  
                    }            
                }
                else if (strlen(arguments[i]) == 2 && arguments[i][0] == '2' && arguments[i][1] == '>') {
                    if (arguments[i+1] != NULL) {
                        file_to_replace_stderr = strdup(arguments[i+1]); 
                        arguments[i] = NULL;
                        arguments[i+1] = NULL;  
                    }
                }
                else valid_arguments++; 
            }            
        }

        int iterator = 0;
        int valid_arguments_counter = 0;  
        char ** sanitized_argument_array = malloc(valid_arguments * sizeof(char *)); 
        while (valid_arguments_counter < valid_arguments) {
            char * argument = *(arguments + iterator);
            if (argument != NULL) {
                sanitized_argument_array[valid_arguments_counter] = strdup(argument); 
                valid_arguments_counter++;
            } 
            iterator++; 
        }

        // now a brand new sanitized array called 'sanitized_argument_array'
        // is available and contains all and only valid arguments. The size of
        // this array is stored in valid_arguments_counter. 

        if ((pid = fork()) == -1) {
            perror("fork - "); 
            exit(1); 
        }

        if (pid == 0) {

            if (file_to_replace_stdin != NULL) {
                int fd = open(file_to_replace_stdin, O_RDONLY); 
                close(STDIN); 
                dup(fd); 
            }
            if (file_to_replace_stdout != NULL) {
                int fd = open(file_to_replace_stdout, O_WRONLY|O_CREAT|O_TRUNC, 0666); 
                close(STDOUT); 
                dup(fd); 
            }
            if (file_to_replace_stderr != NULL) {
                int fd = open(file_to_replace_stdin, O_WRONLY|O_CREAT|O_TRUNC, 0666); 
                close(STDERR); 
                dup(fd); 
            }

            execvp(command_buffer, arguments); 
            fprintf(stderr, "Error in %s execution. \n", command_buffer); 
            exit(2); 
        }
        else if (!background_execution) {
            wait(NULL); 
        }
    }
}