#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#define PLAYER_ONE 1 
#define PLAYER_TWO 2 
#define JUDGE 3
#define SCOREBOARD 4 

// flow: 
// giudice mette nella coda due richieste a p1 e p2
// dopodiche attende due messaggi per lui
// p1 riceve la richiesta e invia la risposta 
// p2 analogo
// giudice processa la vittoria e invia al tabellone 
// giudice controlla se il torneo è finito 
// se è finito invia un sigterm al tabellone che sfrutta
// una procedura di uscita che pone in output il risultato
// FINE. 

typedef struct {
    long type; 
    char text[3]; 
} message; 

void player (int type, int match_queue_descriptor) 
{
    srand(time(NULL) + (type * 15)); 
    short report = 0; 
    message msg; 
    message rcv; 
    
    char ctype = type == 1 ? '1' : '2'; 
    char move[3] = {'S', 'C', 'F'}; 
    char choosed; 

    while (1) {

        report = msgrcv(match_queue_descriptor, &rcv, 3, type, 0);     
        if (report < 0) {
            perror("player msgrcv"); 
            exit(1); 
        }

        choosed = move[rand() % 3];
        msg.type = JUDGE; 
        msg.text[0] = ctype; 
        msg.text[1] = choosed; 
        msg.text[2] = '\0'; 

        printf("player %d sent %c \n", type, choosed); 
        report = msgsnd(match_queue_descriptor, &msg, 3, 0); 
        if (report < 0) {
            perror("player msgsnd"); 
            exit(1); 
        }
    }
    exit(0); 
}

void scoreboard (int score_queue_descriptor)
{
    message rcv; 
    short player_1_points = 0; 
    short player_2_points = 0; 
    short report = 0; 

    while (1) {
        
        report = msgrcv(score_queue_descriptor, &rcv, 3, SCOREBOARD, 0);

        if (rcv.text[1] == 'q') {

            short winner = player_1_points != player_2_points 
                ? (player_1_points > player_2_points ? 1 : 2) : 0; 
            
            if (winner != 0) {
                printf(
                    "[p1:p2 | %d:%d] Player %d won the tournament. \n\n", 
                    player_1_points, 
                    player_2_points, 
                    winner
                );
            }
            else printf(
                "[p1:p2 | %d:%d] The tournament is ended with a draw!\n\n", 
                player_1_points, 
                player_2_points
            );

            exit(0); 
        }
        else if (rcv.text[1] == '1')
            player_1_points ++; 
        else 
            player_2_points ++; 
    }
    exit(0);
}

void main (int argc, char * argv[]) 
{
    short total_matches; 

    if (argc != 2) {
        printf("error, usage: %s <#matches> \n", argv[0]); 
        exit(1); 
    }   

    total_matches = atoi(argv[1]); 
    key_t match_queue_key = ftok("keygen", 32); 
    key_t score_queue_key = ftok("keygen", 33); 

    short match_queue_desc = msgget(match_queue_desc, IPC_CREAT | 0666); 
    short score_queue_desc = msgget(score_queue_desc, IPC_CREAT | 0666); 

    short player_1_pid; 
    short player_2_pid; 
    short scoreboard_pid; 

    if ((player_1_pid = fork()) == 0) {
        player(PLAYER_ONE, match_queue_desc); 
        perror("[player 1 fork]");
        exit(1);   
    }

    if ((player_2_pid = fork()) == 0) {
        player(PLAYER_TWO, match_queue_desc); 
        perror("[player 2 fork]");
        exit(1);   
    }

    if ((scoreboard_pid = fork()) == 0) {
        scoreboard(score_queue_desc); 
        perror("[scoreboard fork]");
        exit(1);   
    }

    short report = 0; 
    short winner = 0; 
    message msg;
    message rcv; 
    char player_1_move = 'C'; 
    char player_2_move = 'C';  

    for (int i = 0; i < total_matches; ++i) { 

        msg.text[0] = '3'; 

        msg.type = PLAYER_ONE; 
        msg.text[1] = 'r';
        msg.text[2] = '\0'; 
        
        report = msgsnd(match_queue_desc, &msg, 2, 0); 
        if (report < 0) {
            perror("[player 1 msgsnd]");
            exit(1); 
        }

        msg.type = PLAYER_TWO; 
        msg.text[0] = 'r';
        msg.text[1] = '\0'; 
        
        report = msgsnd(match_queue_desc, &msg, 2, 0); 
        if (report < 0) {
            perror("[player 1 msgsnd]");
            exit(1); 
        }

        for (int i = 0; i < 2; ++i) {
            report = msgrcv(match_queue_desc, &rcv, 3, JUDGE, 0); 
            
            if (rcv.text[0] == '1') 
                player_1_move = rcv.text[1]; 
            else 
                player_2_move = rcv.text[1]; 
        }

        winner = 0; 
        if (player_1_move != player_2_move) {
            
            if (player_1_move == 'C') 
                winner = player_2_move == 'S' ? 1 : 2; 
            
            else if (player_1_move == 'F')
                winner = player_2_move == 'C' ? 1 : 2; 

            else if (player_1_move == 'S')
                winner = player_2_move == 'F' ? 1 : 2;             
        } 

        if (winner == 0) 
            printf("draw!\n\n"); 
        else {

            printf("player %d won!\n\n", winner); 
            char ctype = winner == 1 ? '1' : '2'; 
            
            msg.type = SCOREBOARD; 
            msg.text[1] = ctype; 
            report = msgsnd(score_queue_desc, &msg, 3, 0); 
            if (report < 0) {
                perror("[scoreboard msgsnd]");
                exit(1); 
            }
        }

        sleep(0.75); 
    } 

    msg.type = SCOREBOARD; 
    msg.text[0] = '3'; 
    msg.text[1] = 'q'; 
    msg.text[2] = '\0'; 

    report = msgsnd(score_queue_desc, &msg, 3, 0); 
    if (report < 0) {
        perror("[scoreboard msgsnd]");
        exit(1); 
    }

    sleep(1.25);
    msgctl(match_queue_desc, IPC_RMID, NULL);     
    msgctl(score_queue_desc, IPC_RMID, NULL);        

    kill(player_1_pid, SIGTERM); 
    kill(player_2_pid, SIGTERM); 
    kill(scoreboard_pid, SIGTERM); 
}