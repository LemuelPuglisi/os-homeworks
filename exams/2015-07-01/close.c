#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>

int main (int argc, char * argv[]) 
{
    key_t c_msg_queue_key = ftok("key/secure_ipc_key", 32); 
    key_t d_msg_queue_key = ftok("key/secure_ipc_key", 64); 

    short c_msg_queue_descriptor = msgget(c_msg_queue_key, IPC_CREAT | 0660); 
    short d_msg_queue_descriptor = msgget(d_msg_queue_key, IPC_CREAT | 0660); 

    msgctl(c_msg_queue_descriptor, IPC_RMID, NULL); 
    msgctl(d_msg_queue_descriptor, IPC_RMID, NULL); 
}