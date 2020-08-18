#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

void throwPerror(char *source)
{
    char str[256]; 
    sprintf(str, "[error | %s]", source); 
	perror(str);
	exit(-1);
}

void throwErrorWithMessage(char *message)
{
    fprintf(stderr, "[error] %s \n", message); 
	exit(-1);
}

void swap_memory(void * a, void * b, int bytes)
{
    char t[bytes]; 
    memcpy(t, a, bytes);
    memcpy(a, b, bytes); 
    memcpy(b, t, bytes); 
}

void bubble_sort (char * array, size_t size, short bytes_in_line)
{
    int records = size / bytes_in_line; 
    for (int i = 0; i < records; ++i) {
        for (int j = i; j < records; ++j) {

            // get the first portion of memory 
            char * a = array + (i * bytes_in_line); 
            // get the second portion of memory
            char * b = array + (j * bytes_in_line); 

            if (memcmp(a, b, bytes_in_line) > 0) // then a > b
            {
                swap_memory(a, b, bytes_in_line);  
            }
        }
    }
}

int main (int argc, char * argv[]) 
{
    int     bytes_in_line, 
            records; 
    char    * file_path, 
            * array; 
    int     file_descriptor; 
    
    struct stat filestat; 

    if (argc == 3) 
    {
        file_path = argv[1]; 
        bytes_in_line = atoi(argv[2]) + 1; // also counting \n character  
    }    
    else throwErrorWithMessage("usage: sort <file path> <bytes per line>");

    printf("file path is %s \n", file_path);

    if (stat(file_path, &filestat) == -1) 
    {
        throwPerror("fstat"); 
    }  

    if ((file_descriptor = open(file_path, O_RDWR)) < 0) 
    {
        throwPerror("open"); 
    }

    array =
		mmap(NULL, filestat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0); 

    if (array == MAP_FAILED) 
    {
        throwPerror("mmap"); 
    }

    records = filestat.st_size / bytes_in_line;
    
    bubble_sort(array, filestat.st_size, bytes_in_line); 

    // @debug (fastest way to for loop)
    size_t size = filestat.st_size; 
    for (char * temp = array; temp != &array[size]; ++temp)
        printf("%c", *temp); 

    munmap(array, filestat.st_size);
}