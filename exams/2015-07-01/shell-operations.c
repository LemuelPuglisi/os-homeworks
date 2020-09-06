/**
 * TODO:    implementare le operazioni e inserire l'output nella coda 
 *          dei dati. Dopodiché far leggere al processo padre tutti i dati. 
 *          Quando un risultato non rientra in 1KB, segnare l'ultimo carattere
 *          con un carattere speciale. Questo riferirà al padre che il risultato
 *          non è concluso e che deve continuare a prelevare risultati dalla coda. 
 * 
 * OPPURE:  dato che viene eseguito un comando alla volta, non è necessario dove 
 *          utilizzare un carattere speciale. Potremmo benissimo suddividere il 
 *          risultato in blocchi da 1024 byte. 
 *  
 */ 

#include <dirent.h>

/**
 * Prende in input il path di una cartella. 
 * Ritorna un array di stringhe terminato da un puntatore a NULL.
 * Ogni stringa rappresenta uno dei file regolari contenuti
 * all'interno della cartella in input. 
 */ 
char ** get_regular_files_from_directory (char * directory_path)
{
    DIR * directory = opendir(directory_path);
    struct dirent * dir; 

    if (directory) {
        while ((dir = readdir(directory)) != NULL) {
            
        }
        closedir(directory); 
    }
    
}