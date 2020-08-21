#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include <string.h>
#include <strings.h>
#include <libgen.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

/**
 * throw errno error specifying the error source 
 * 
 * @param char * source
 */  
void throw_perror(char *source)
{
    char * str = malloc(strlen(source) + 15); 
    sprintf(str, "[error | %s]", source); 
    perror(str);
	exit(-1);
}

/**
 * terminate the program whit a message 
 * 
 * @param char * message
 */  
void throw_error_with_message(char *message)
{
    fprintf(stderr, "[error] %s \n", message); 
	exit(-1);
}

/**
 * take the input resource path and extract the name. Then 
 * append this name in the destination_base string. For 
 * example: 
 * 
 * resource_path:       /var/log/test.log
 * destination_base:    /home/log/
 * output:              /home/log/test.log
 * 
 * @param char * message
 * @return char * resource rebased 
 */ 
char * move_resource_base (char * resource_path, char * destination_base)
{
    char * filename = basename(resource_path); 
    char * resource_rebased = malloc(strlen(filename) + strlen(destination_base)); 
    strcat(resource_rebased, destination_base); 

    if (destination_base[strlen(destination_base) - 1] != '/') 
        strcat(resource_rebased, "/");    

    strcat(resource_rebased, filename);
    return resource_rebased;  
}

/*
    Homework n.2

    Estendere l'esempio 'move.c' visto a lezione per supportare i 2 casi speciali:
    -   spostamento cross-filesystem: individuato tale caso, il file deve essere
        spostato utilizzando la strategia "copia & cancella";
    -   spostamento di un link simbolico: individuato tale caso, il link simbolico
        deve essere ricreato a destinazione con lo stesso contenuto (ovvero il percorso
        che denota l'oggetto referenziato); notate come tale spostamento potrebbe
        rendere il nuovo link simbolico non effettivamente valido.

    La sintassi da supportare e' la seguente:
     $ homework-2 <pathname sorgente> <pathname destinazione>
*/

int main (int argc, char* argv[])  {

    struct stat srcinfo, dstinfo; 
    char *srcpath, *dstpath; 
    int same_file_system = 1, 
        is_symbolic_link = 0; 

    if (argc == 3) {
        srcpath = argv[1]; 
        dstpath = argv[2]; 
    }
    else throw_error_with_message("usage: move <source pathname> <destination pathname>"); 

    if (lstat(srcpath, &srcinfo) == -1 || stat(dstpath, &dstinfo) == -1) {
        throw_perror("stat"); 
    }

    // get the new file path 
    char * dst_file_path = move_resource_base(srcpath, dstpath); 

    if (S_ISLNK(srcinfo.st_mode)) {
        
        // moving a symlink 
        const short link_content_buffer_size = 2048; 
        char link_content_buffer[link_content_buffer_size]; 
        readlink(srcpath, link_content_buffer, link_content_buffer_size);
        symlink(link_content_buffer, dst_file_path);  
        unlink(srcpath);

    }
    else if (srcinfo.st_dev == dstinfo.st_dev) {

        // same file system, using rename function. 
        rename(srcpath, dst_file_path); 

    }
    else {

        // regular file, different file system. Must copy and delete. 
        const short buffer_dim = 1024;
        char buffer[buffer_dim];
        
        FILE * src_file = fopen(srcpath, "r");
        FILE * dst_file = fopen(dst_file_path, "w+"); 
         
        while (fgets(buffer, buffer_dim - 1, src_file))
            fprintf(dst_file, "%s", buffer); 
        
        fclose(src_file); 
        fclose(dst_file);
        unlink(srcpath); 
    }

}