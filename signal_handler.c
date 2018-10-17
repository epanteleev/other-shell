#include "signal_handler.h"
#include "common.h"


void signal_handler_child(int p)
{
    int status;
    pid_t pid;
   // fprintf(stderr,"\a\a\a\a\a\a\a");
    while((pid=waitpid(-1,&status,WNOHANG))>0){}
}

void signal_handler_int(int p){
    if (kill(CURRENT_PID,SIGTERM) == 0){
        fprintf(stderr, "\n\x1b[1;35mProcess\x1b[0m \x1b[1;31m%d \x1b[0m \x1b[1;35mreceived a SIGINT signal\x1b[0m\n",CURRENT_PID);
    }
    else{
        fprintf(stderr,"\n");
        shell_prompt();
    }
}

void shell_prompt(){
    char hostname[1024];
    gethostname(hostname, sizeof(hostname));
    fprintf(stderr,"\x1b[1;31m%s@%s\x1b[0m$ \x1b[1;35m%s\x1b[0m$ ", getenv("LOGNAME"), hostname, getcwd(current_directory, 1024));
}
