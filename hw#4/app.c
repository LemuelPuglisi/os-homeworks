#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

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

/**
 * Copy the input file (given a path) to a destination folder
 * 
 * @param char * file_path
 * @param char * destination_dir
 * @return void 
 */ 
void copy_file (char * file_path, char * destination_dir)
{
    const short buffer_dim = 1024; 
    char * destination_file_path = move_resource_base(file_path, destination_dir);

    FILE * src_file = fopen(file_path, "r");
    FILE * dst_file = fopen(destination_file_path, "w+"); 

    char buffer[buffer_dim]; 
    while (fgets(buffer, buffer_dim - 1, src_file))
        fprintf(dst_file, "%s", buffer); 
    
    fclose(src_file); 
    fclose(dst_file); 
}

/**
 * Copy the input symbolic link (given a path) to a destination folder
 * 
 * @param char * file_path
 * @param char * destination_dir
 * @return void 
 */ 
void copy_symlink (char * symlink_path, char * destination_dir)
{
    const short link_content_buffer_size = 2048; 
    char link_content_buffer[link_content_buffer_size]; 

    char * destination_symlink_path = move_resource_base(symlink_path, destination_dir); 

    readlink(symlink_path, link_content_buffer, link_content_buffer_size); 
    symlink(link_content_buffer, destination_symlink_path); 
}

/**
 * Recursively copy the content of a given directory to a destination directory. 
 * 
 * @param char * dir_path
 * @param char * destination_dir
 * @return void 
 */
void copy_dir_content (char * dir_path, char * destination_dir)
{
    DIR * current_dir = opendir(dir_path); 
    if (!current_dir) return; 

    struct dirent * current_resource; 

    while (current_resource = readdir(current_dir)) {

        char * full_resource_path = move_resource_base(current_resource->d_name, dir_path);

        if (current_resource->d_type == DT_REG) {
            copy_file(full_resource_path, destination_dir); 
        }
        else if (current_resource->d_type == DT_LNK) {
            copy_symlink(full_resource_path, destination_dir); 
        }
        else if (current_resource->d_type == DT_DIR && current_resource->d_name[0] != '.') {
                       
            struct stat resource_stat; 
            stat(full_resource_path, &resource_stat); 

            // 1. build a path appending the dir name to the destination dir
            // 2. create a brand new directory using this path 
            // 3. call the procedure again

            char * new_destination_dir = move_resource_base(current_resource->d_name, destination_dir); 
            mkdir(new_destination_dir, resource_stat.st_mode);  
            copy_dir_content(full_resource_path, new_destination_dir); 
        }
    }
}

/*
    Homework n.4

    Estendere l'esercizio 'homework n.1' affinche' operi correttamente
    anche nel caso in cui tra le sorgenti e' indicata una directory, copiandone
    il contenuto ricorsivamente. Eventuali link simbolici incontrati dovranno
    essere replicati come tali (dovrà essere creato un link e si dovranno
    preservare tutti permessi di accesso originali dei file e directory).

    Una ipotetica invocazione potrebbe essere la seguente:
     $ homework-4 directory-di-esempio file-semplice.txt path/altra-dir/ "nome con spazi.pdf" directory-destinazione
*/

int main (int argc, char * argv[])
{
    if (argc < 3) {
        throw_error_with_message("usage: program <file|directory, ...> <destination>"); 
    }

    int number_of_resources_to_copy = argc - 2; 
    char * destination_folder = argv[argc - 1]; 
    struct stat destination_folder_stat; 

    if (lstat(destination_folder, &destination_folder_stat) == -1) {
        throw_perror("stat"); 
    }

    if (!S_ISDIR(destination_folder_stat.st_mode)) {
        throw_error_with_message("destination must be a valid directory."); 
    }

    struct stat current_resource_stat; 

    for (int i = 0; i < number_of_resources_to_copy; ++i) {

        // the first argument is the program name
        char * current_resource = argv[1+i]; 

        if (lstat(current_resource, &current_resource_stat) == -1) {
            throw_perror("stat"); 
        }

        switch (current_resource_stat.st_mode & S_IFMT) {
           
           case S_IFDIR:  
            copy_dir_content(current_resource, destination_folder);      
            break;
           
           case S_IFREG:  
            copy_file(current_resource, destination_folder);            
            break;
           
           case S_IFLNK:  
            copy_symlink(current_resource, destination_folder);              
            break;

           default:       
            fprintf(stderr, "cannot copy the following file: %s \n", current_resource);                
            break;

        }
    }
}
