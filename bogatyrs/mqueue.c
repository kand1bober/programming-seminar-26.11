#include "../include/mqueue.h"

int get_msg(int qid, int msg_type, struct msgbuf* msg)
{
    if (msgrcv(qid, msg, sizeof(msg->mtext), msg_type,
        MSG_NOERROR | IPC_NOWAIT) < 0) {
        if (errno != ENOMSG) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        else {
            return -1; //no messages
            // printf("No message available for msgrcv()\n");
        }
    } 

    return 0;
}


void send_msg(int qid, int msg_type, char* text)
{
    struct msgbuf msg;
    msg.mtype = msg_type;
    snprintf(msg.mtext, sizeof(msg.mtext), "%s", text);
    
    // send msg to queue
    if (msgsnd(qid, &msg, sizeof(msg.mtext), IPC_NOWAIT) < 0) {
        perror("error msgsnd");
        exit(EXIT_FAILURE);
    }
}
