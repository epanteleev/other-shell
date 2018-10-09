#include "signal_handler.h"
#include "common.h"


void signal_handler_child(int p)
{
    while(waitpid(-1, NULL, WNOHANG) > 0)
    {
        ///printf("hrhrhr\n");
    }
}

void signal_handler_int(int p)
{
    // посылаем сигнал SIGTERM ребенку
    if (kill(CURRENT_PID,SIGTERM) == 0)
    {
        fprintf(stderr, "\n\x1b[1;35mProcess\x1b[0m \x1b[1;31m%d \x1b[0m \x1b[1;35mreceived a SIGINT signal\x1b[0m\n",CURRENT_PID);
        NO_PRINT = 1;
    }
    else
    {
        printf("\n");
    }
}

void signal_handler_hup(int p)
{
    // посылаем сигнал SIGTERM ребенку
    if (kill(CURRENT_PID,SIGTERM) == 0)
    {
        fprintf(stderr, "\n\x1b[1;35mProcess\x1b[0m \x1b[1;31m%d \x1b[0m \x1b[1;35mreceived a SIGHUP signal\x1b[0m\n",CURRENT_PID);
    }
    else
    {
        printf("\n");
    }
}
void signal_handler_tstp(int p )
{
    // посылаем сигнал SIGTERM ребенку
    if (kill(CURRENT_PID,SIGTSTP) == 0)
    {
        fprintf(stderr,"\nProcess %d received a SIGTSTP signal\n",CURRENT_PID);
    }
    else
    {
        printf("\n");
    }
}
void signal_handler_quit(int p)
{
    if (kill(CURRENT_PID,SIGQUIT) == 0){
        fprintf(stderr,"\n\x1b[1;35mProcess\x1b[0m \x1b[1;31m%d \x1b[0m \x1b[1;35mreceived a SIGHUP signal\x1b[0m\n",CURRENT_PID);
        NO_PRINT = 1;
    }else{
        printf("\n");
    }
}
