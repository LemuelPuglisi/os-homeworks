#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#define MIN_PARAMS 3
#define MAX_FILE_NAME 20
#define BUFFER_DIM 1000

/**
 * Function extract filename 
 * extracts filename from a given path 
 * and puts it into filename variable
 * @param char * path 
 * @param char * filename  
 * @return void 
 */

void extract_filename (char * path, char * filename); 

/*
    Homework n.1

    Scrivere un programma in linguaggio C che permetta di copiare un numero
    arbitrario di file regolari su una directory di destinazione preesistente.

    Il programma dovra' accettare una sintassi del tipo:
     $ homework-1 file1.txt path/file2.txt "nome con spazi.pdf" directory-destinazione
*/

int main (int argc, char* argv[])
{
    char *  directory; 
    char *  filename_src; 
    char    filename_dest[256]; 
    char    destionatio_path[1000]; 
    char    copy_buffer[BUFFER_DIM]; 
    int     cmd_response = 0, 
            number_of_files; 
 
    FILE *  source, 
         *  destination; 
    DIR  *  dir; 

    if (argc < MIN_PARAMS) {

        printf("[error] usage: %s <file_1 ... file_n> <directory>\n", argv[0]); 
        exit(1); 
    }

    directory = argv[argc - 1]; 

    if ((dir = opendir(directory)) == NULL) {

        printf("[error] the directory %s doesn't exists.\n", directory); 
        exit(1); 
    }

    number_of_files = argc - 2;

    for (int i = 0; i < number_of_files; ++i) {
            
        filename_src = argv[i + 1];
        extract_filename(filename_src, filename_dest);

        strcpy(destionatio_path, directory); 
        strcat(destionatio_path, "/"); 
        strcat(destionatio_path, filename_dest); 

        if ((source = fopen (filename_src, "r")) == NULL) {

            printf("[error] the file %s doesn't exists.\n", filename_src); 
            exit(1); 
        }

        destination = fopen (destionatio_path, "w"); 
         
        memset(copy_buffer, 0, BUFFER_DIM);

        do {
                            
            cmd_response = fread(copy_buffer, 200, 5, source);  
            fwrite(copy_buffer, 200, cmd_response, destination); 

        } while (cmd_response > 0); 

        fclose(source); 
        fclose(destination); 
    }

}


void extract_filename (char * path, char * filename) 
{
    size_t len = strlen(path);
    int char_counter = 0, start_position = 0; 

    for (int i = len - 1; i >= 0; --i) {
        if (path[i] == '/') {
            break; 
        }
        else {
            ++char_counter; 
        } 
    }

    start_position = len - char_counter; 
    strncpy (filename, path + start_position, char_counter); 
    filename[char_counter] = '\0'; 
}