#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#define JUDGE_PROCESS_TYPE 1

typedef struct {
    long type; 
    char text[3]; 
} message;

short get_player_type (short player_number) 
{
    return 1 + JUDGE_PROCESS_TYPE + player_number;   
}

short get_player_number (short player_type) 
{
    return player_type - 1 - JUDGE_PROCESS_TYPE; 
}

void player(short number, short message_queue_descriptor)
{
    srand(time(NULL) + (number * 17)); 

    message msg; 
    short report; 
    short player_type = get_player_type(number); 
    short move = 0;

    while (1) {
        
        report = msgrcv(message_queue_descriptor, &msg, 3, player_type, 0); 
        if (report < 0) {
            perror("msgrcv"); 
            exit(EXIT_FAILURE);
        }

        if (msg.text[1] == 'q') {
            break; 
        }

        move = rand() % 10; 
        msg.type = JUDGE_PROCESS_TYPE; 
        msg.text[0] = number + '0'; 
        msg.text[1] = move + '0'; 
        msg.text[2] = '\0'; 

        report = msgsnd(message_queue_descriptor, &msg, 3, 0); 
        if (report < 0) {
            perror("msgsnd"); 
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS); 
}

int main(int argc, char * argv[])
{
    short players;
    short matchs; 

    if (argc != 3) {
        fprintf(stderr, "error, usage: %s <players> <matchs> \n", argv[0]); 
        exit(EXIT_FAILURE); 
    }

    players = atoi(argv[1]);
    matchs  = atoi(argv[2]); 

    if (players < 2 || players > 6) {
        fprintf(stderr, "error, player number must be between 2 and 6. \n"); 
        exit(EXIT_FAILURE); 
    }

    if (matchs < 1) {
        fprintf(stderr, "error, play at least 1 match. \n"); 
        exit(EXIT_FAILURE); 
    }


    short scoreboard[players];
    bzero(scoreboard, sizeof(short) * players); 

    short moves[players]; 
    bzero(moves, sizeof(short) * players); 
    
    short message_queue_descriptor = msgget(IPC_PRIVATE, IPC_CREAT | 0666); 
    if (message_queue_descriptor < 0) {
        perror("msgget"); 
        exit(EXIT_FAILURE); 
    }

    for (short i = 0; i < players; ++i) {
        if (fork() == 0) {
            player(i, message_queue_descriptor); 
            perror("fork player"); 
            exit(EXIT_FAILURE); 
        }
    }

    message msg; 
    short report = 0; 
    short draw = 0; 


    for (short match = 0; match < matchs; ++match) {

        draw = 0; 

        // reset player moves 
        bzero(moves, sizeof(short) * players); 

        // send as async request to other processes 
        for (short player = 0; player < players; ++player) { 
            msg.type = get_player_type(player); 
            msg.text[0] = '1'; 
            msg.text[1] = 'r'; 
            msg.text[2] = '\0'; 

            report = msgsnd(message_queue_descriptor, &msg, 3, 0); 
            if (report < 0) {
                perror("msgsnd"); 
                exit(EXIT_FAILURE); 
            }
        }

        // require other process responses 
        for (short i = 0; i < players; ++i) { 

            report = msgrcv(message_queue_descriptor, &msg, 3, JUDGE_PROCESS_TYPE, 0); 
            if (report < 0) {
                perror("msgrcv"); 
                exit(EXIT_FAILURE); 
            }

            char rcv_plyr[2] = { msg.text[0], '\0' }; 
            char rcv_move[2] = { msg.text[1], '\0' }; 

            short sender_player  = atoi(rcv_plyr); 
            moves[sender_player] = atoi(rcv_move); 
            printf("player %d: %d \n", sender_player, moves[sender_player]);
        }

        // check if the match is draw
        for (int i = 0; i < players && draw != 1; ++i) {
            for (int j = i + 1; j < players; ++j) {
                if (moves[i] == moves[j]) {
                    draw = 1; 
                    break; 
                }
            } 
        }

        if (draw == 1) {
            printf("the match is ended with a draw. \n"); 
        }
        else {
            
            short acc = 0; 
            for (int i = 0; i < players; ++i) acc += moves[i]; 

            short winner = acc % players; 
            printf("the winner of this match is player %d \n", winner); 
            scoreboard[winner]++; 
        }

    }

    // close all the processes 
    for (short player = 0; player < players; ++player) { 
        msg.type = get_player_type(player); 
        msg.text[0] = '1'; 
        msg.text[1] = 'q'; 
        msg.text[2] = '\0'; 

        report = msgsnd(message_queue_descriptor, &msg, 3, 0); 
        if (report < 0) {
            perror("msgsnd"); 
            exit(EXIT_FAILURE); 
        }
    }

    int winner = 0; 
    for (int i = 1; i < players; ++i) 
        if (scoreboard[winner] < scoreboard[i])
            winner = i; 
    
    printf("the winner is player %d \n", winner); 
    msgctl(message_queue_descriptor, IPC_RMID, NULL); 
    exit(EXIT_SUCCESS); 
}