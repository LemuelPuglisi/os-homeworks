#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>

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

    struct stat *srcinfo, *tgtinfo; 
    char *srcpath, *tgtpath; 
    int cmdresponse = 0; 

    if (argc != 3) {
        printf("usage: %s <source pathname> <destination pathname>\n", argv[0]); 
        exit(1); 
    } 
    else {
        srcpath = argv[1]; 
        tgtpath = dirname(argv[2]); 
    }

    cmdresponse |= lstat(srcpath, srcinfo) * -1;
    cmdresponse |=  stat(tgtpath, tgtinfo) * -1;

    if (cmdresponse == 1) {
        printf("[error] invalid path\n"); 
        exit(1); 
    }

    // controllo il file system di entrambi i path
    if (srcinfo->st_dev == tgtinfo->st_dev) {

        // hard link copy and remove original one
        printf("same file system \n"); 
    }
    else {

        // different file system 
        printf("diff file system \n"); 
    }
}