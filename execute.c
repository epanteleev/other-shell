#include "execute.h"

static void file_io(command* cmd){
    if(cmd == NULL){
        fprintf(stderr, "incorrect argument\n");
        return;
    }
    int fileDescriptor;
    if (cmd->file_out != NULL){
        fileDescriptor = open(cmd->file_out, O_CREAT | O_TRUNC | O_WRONLY, 0600);
        if(fileDescriptor == -1){
            fprintf(stderr,"File didn't open\n");
            return;
        }
        dup2(fileDescriptor, STDOUT_FILENO);
        close(fileDescriptor);
    }
    if (cmd->file_in != NULL){
        fileDescriptor = open(cmd->file_in, O_RDONLY, 0600);
        if(fileDescriptor == -1){
            fprintf(stderr,"File didn't open\n");
            return;
        }
        dup2(fileDescriptor, STDIN_FILENO);
        close(fileDescriptor);
    }
}

static int waiting(process_list* process,size_t num_process, pid_t pgid){
    int res = 0;
    for(size_t i = 0;i < num_process;i++){
        int status = 0;
        pid_t p = waitpid(-pgid,&status,WUNTRACED);
        if(WIFSTOPPED(status)){
            if(p == pgid){
                res = 1;
            }
        }else {
            process_list_remove(process,p);
        }
    }
    return res;
}

void execute(command_list* head, process_list* process){
    if(head == NULL){
        return;
    }
    command* cmd = head->cmd;
    pid_t pid;
    size_t id = 0;
    if((pid = fork()) == -1){
        perror("Child process could not be created\n");
        return;
    }
    if(pid == 0){
        signal (SIGINT, SIG_DFL);
        signal (SIGQUIT, SIG_DFL);
        signal (SIGTSTP, SIG_DFL);
        signal (SIGTTIN, SIG_DFL);
        signal (SIGTTOU, SIG_DFL);
        signal (SIGCHLD, SIG_DFL);
        setpgid(0,0);
        file_io(cmd);
        if (execvp(cmd->tokens[0],cmd->tokens)==-1){
            perror( COLOR_YELLOW "Command not found" COLOR_ALL_STRING);
            _exit(1);
        }
    }
    if(pid != 0){
       id = process_list_insert(process,1,pid);
    }
    if (head->background == 0){
        signal (SIGTTOU, SIG_IGN);
        if(tcsetpgrp(STDIN_FILENO,pid) == -1){
            perror("tcsetpgrp");
            return;
        }
        signal (SIGTTOU, SIG_DFL);
        int status = 0;
        waitpid(pid,&status,WUNTRACED);
        if(WIFSTOPPED(status) ){
            fprintf(stderr,"[%ld] Stopped...\n",id);
        }else {
            process_list_remove(process,pid);
        }
        signal (SIGTTOU, SIG_IGN);
        if(tcsetpgrp(STDIN_FILENO,getpid()) == -1){
            perror("tcsetpgrp");
            return;
        }
        signal (SIGTTOU, SIG_DFL);
    }else{
        fprintf(stderr,"[%ld] Created...\n",id);
    }
}

int piped_execute(command_list* head, process_list* process){
    if(head == NULL || process == NULL){
        return -1;
    }
    size_t pnum = head->sizelist * 2;
    int* pipes = (int*) malloc(sizeof(int)*head->sizelist*2);
    if(pipes == NULL){
        fprintf(stderr, "memory wasn't allocated");
        return 1;
    }
    for(size_t i = 0; i < pnum;i+=2){
        if(pipe(pipes+i) == -1){
            perror("pipe error");
            free(pipes);
            return -1;
        }
    }
    sigset_t set;
    lock_sigchld(&set);
    command *temp = head->cmd;
    pid_t pid, pgid;
    size_t commands = head->sizelist;
    size_t  id = 0;
    for(size_t i = 0; i < commands; i++){
        pid = fork();
        if( temp->next == NULL || temp->next->numtokens == 0){
            id = process_list_insert(process,commands,pgid);
        }
        if(i == 0 && pid != 0){
            pgid = pid;
        }
        if(pid != 0){
            setpgid(pid,pgid);
        }
        if(pid == 0){
            signal (SIGINT, SIG_DFL);
            signal (SIGQUIT, SIG_DFL);
            signal (SIGTSTP, SIG_DFL);
            signal (SIGTTIN, SIG_DFL);
            signal (SIGTTOU, SIG_DFL);
            signal (SIGCHLD, SIG_DFL);
            if(temp->next!=NULL && temp->next->numtokens != 0){
                if(dup2(pipes[2*i+1],STDOUT_FILENO) == -1){
                    perror("dup2 error");
                }
            }
            file_io(temp);
            if(i != 0){
                if(dup2(pipes[2*(i-1)],STDIN_FILENO) == -1){
                    perror("dup2 error");
                }
            }
            for(size_t k = 0;k < pnum - 2;k++){
                close(pipes[k]);
            }
            free(pipes);
            if(execvp(temp->tokens[0],temp->tokens) == -1){
                perror("Cannot execvp");
                _exit(-1);
            }
        }
        else if(pid < 0){
            perror("Could not fork child");
            free(pipes);
            return -1;
        }
        temp=temp->next;
    }

    for(size_t i=0;i < pnum-2;i++){
        close(pipes[i]);							//close all pipes in parent
    }
    free(pipes);
    if(head->background == 0){
        if(tcsetpgrp(STDIN_FILENO,pgid) == -1){
            perror("tcsetpgrp");
            return -1;
        }
        if(waiting(process, head->sizelist, pgid)){
            fprintf(stderr,"[%ld] Stopped...\n",id);
        }
        signal (SIGTTOU, SIG_IGN);
        if(tcsetpgrp(STDIN_FILENO,getpid()) == -1){
            perror("tcsetpgrp");
            return -1;
        }
        signal (SIGTTOU, SIG_DFL);
    }else{
        fprintf(stderr,"[%ld] Created...\n",id);
    }
    unlock_sigchld(&set);
    return 0;
}

int launch_stopped_prog(process_list* process, char* pid_, int back){
    if(pid_ == NULL){
        return EXIT_FAILURE;
    }
    size_t pid = (size_t)atoi(pid_);
    if(pid == 0){
        perror("Call atoi");
        return EXIT_FAILURE;
    }
    sigset_t set;
    lock_sigchld(&set);
    jobs* jb = process_list_get(process, pid);
    if(jb == NULL){
        return EXIT_FAILURE;
    }
    if(killpg(jb->pgid,SIGCONT) == -1){
        return EXIT_FAILURE;
    }
    if(back == 0){
        if(tcsetpgrp(STDIN_FILENO,jb->pgid) == -1){
            perror("tcsetpgrp");
            return -1;
        }
        if(waiting(process,jb->depth,jb->pgid)){
            fprintf(stderr,"[%ld] Stopped...\n",jb->id);
        }
        signal (SIGTTOU, SIG_IGN);
        if(tcsetpgrp(STDIN_FILENO,getpid()) == -1){
            perror("tcsetpgrp");
            return -1;
        }
    }else{
        fprintf(stderr,"[%ld] Created...\n",jb->id);
    }
    unlock_sigchld(&set);
    return EXIT_SUCCESS;
}
