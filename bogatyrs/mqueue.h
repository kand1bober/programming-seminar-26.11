#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/time.h>

#ifndef MQUEUE_HEADER
#define MQUEUE_HEADER

#define MSG_SIZE 64

struct msgbuf {
    long mtype; /* message type, must be > 0 */
    char mtext[MSG_SIZE]; /* message data */
};

int get_msg(int qid, int msg_type, struct msgbuf* msg);

void send_msg(int qid, int msg_type, char* text);

#endif
