#include "signal_handler.h"
#include "common.h"


void signal_handler_child(int p){
    int status;
    sigset_t set;

    lock_sigchld(&set);
    while(waitpid(-1,&status,WNOHANG)>0){}

    unlock_sigchld(&set);
}

int lock_sigchld(sigset_t* set){
    if (sigemptyset(set)) {
        return -1;
    }
    if (sigaddset(set, SIGCHLD)) {
        return -1;
    }
    if (sigprocmask(SIG_BLOCK, set, NULL)) {
        return -1;
    }
    return 0;
}

int unlock_sigchld(sigset_t* set){
    if (sigprocmask(SIG_UNBLOCK, set, NULL)) {
        return -1;
    }
    return 0;
}


