#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define S_MUTEX_READ 0
#define S_MUTEX_PROC 1
#define S_MUTEX_WRIT 2

#define MAX_SEG_DIM 64

void WAIT (short semaphore_id, short semaphore) 
{
    struct sembuf operation; 
    operation.sem_flg = 0; 
    operation.sem_num = semaphore; 
    operation.sem_op = -1;
    semop(semaphore_id, &operation, 1); 
}

void SIGNAL (short semaphore_id, short semaphore)
{
    struct sembuf operation; 
    operation.sem_flg = 0; 
    operation.sem_num = semaphore; 
    operation.sem_op = +1;
    semop(semaphore_id, &operation, 1); 

}

void reader(char * input_path, short shared_memory_id, short semaphore_id)
{
    char * shared_word = shmat(shared_memory_id, NULL, 0);
    FILE * file_pointer; 
    size_t bytes_read = 0; 
    size_t len = 0; 
    char * line; 

    file_pointer = fopen(input_path, "r"); 
    if (file_pointer == NULL) {
        exit(1); 
    }

    while ((bytes_read = getline(&line, &len, file_pointer)) != -1) {
        
        line[bytes_read - 1] = '\0';
        WAIT(semaphore_id, S_MUTEX_READ); 
        memcpy(shared_word, line, bytes_read);
        SIGNAL(semaphore_id, S_MUTEX_PROC); 
    }

    WAIT(semaphore_id, S_MUTEX_READ); 
    memcpy(shared_word, "\0", MAX_SEG_DIM);
    SIGNAL(semaphore_id, S_MUTEX_PROC); 

    fclose(file_pointer);
    exit(0); 
}

void writer(char * output_path, short shared_memory_id, short semaphore_id)
{
    char * shared_word = shmat(shared_memory_id, NULL, 0); 
    FILE * file_descriptor = output_path == NULL ? stdout : fopen(output_path, "w+"); 

    while (1) {

        WAIT(semaphore_id, S_MUTEX_WRIT);

        if (shared_word[0] == '\0') {
            SIGNAL(semaphore_id, S_MUTEX_PROC);
            fclose(file_descriptor); 
            exit(0); 
        }

        fprintf(file_descriptor, "%s\n", shared_word); 
        SIGNAL(semaphore_id, S_MUTEX_PROC); 
    }
    exit(0);
}

int is_word_palindrome (char * word) 
{
    short len = strlen(word); 
    for (int i = 0; i < len / 2; i++) {

        short e = len-1-i; 
        char a = word[i] > 96 ? word[i] - 32 : word[i]; 
        char b = word[e] > 96 ? word[e] - 32 : word[e];
        if (a != b) return 0; 
    }
    return 1; 
}

int main (int argc, char * argv[])
{
    char * input_path; 
    char * output_path; 
    short semaphore_id; 
    short shared_memory_id; 

    if (argc < 2) {
        printf("[error] usage: %s <input file> [output file] \n", argv[0]); 
        exit(1); 
    }

    input_path = argv[1]; 
    output_path = argc > 2 ? argv[2] : NULL; 

    semaphore_id = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666); 
    if (semaphore_id < 0) {
        perror("[semget]");
        exit(1); 
    }

    semctl(semaphore_id, S_MUTEX_READ, SETVAL, 1);
    semctl(semaphore_id, S_MUTEX_PROC, SETVAL, 0);
    semctl(semaphore_id, S_MUTEX_WRIT, SETVAL, 0);

    shared_memory_id = shmget(
        IPC_CREAT, 
        MAX_SEG_DIM * sizeof(char), 
        IPC_CREAT | 0666
    );

    if (shared_memory_id < 0) {
        perror("[shmget]");
        exit(1); 
    }

    if (fork() == 0) {
        reader(input_path, shared_memory_id, semaphore_id); 
        perror("[fork reader]"); 
        exit(1); 
    }

    if (fork() == 0) {
        writer(output_path, shared_memory_id, semaphore_id); 
        perror("[fork writer]"); 
        exit(1); 
    }

    char * shared_word = shmat(shared_memory_id, NULL, 0); 

    while (1) {

        WAIT(semaphore_id, S_MUTEX_PROC);

        if (shared_word[0] == '\0') {
            SIGNAL(semaphore_id, S_MUTEX_WRIT); 
            WAIT(semaphore_id, S_MUTEX_PROC); 
            break; 
        }

        if (is_word_palindrome(shared_word)) {
            SIGNAL(semaphore_id, S_MUTEX_WRIT); 
            WAIT(semaphore_id, S_MUTEX_PROC); 
        }
        
        SIGNAL(semaphore_id, S_MUTEX_READ); 
    }

    shmctl(shared_memory_id, IPC_RMID, 0); 
    semctl(semaphore_id, 0, IPC_RMID, 0); 
    exit(0); 
}