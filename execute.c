#include "execute.h"

static int waiting(process_list* process,size_t num_process, pid_t pgid){
    for(size_t i = 0;i < num_process;i++){
        int status = 0;
        pid_t p = waitpid(-pgid,&status,WUNTRACED | WEXITED);
        //perror("here");
        //fprintf(stderr,"%d\n",pgid);

        if(WIFSTOPPED(status)){
            if(p == pgid){
                fprintf(stderr,"\nProcess %d received a SIGTSTP signal\n",p);
            }
        }else {
            process_list_remove(process,p);
        }
    }
    return 0;
}

static void file_io(command* cmd){
    if(cmd == NULL){
        fprintf(stderr, "incorrect argument\n");
        return;
    }
    int fileDescriptor;
    if (cmd->file_out != NULL)//write
    {
        fileDescriptor = open(cmd->file_out, O_CREAT | O_TRUNC | O_WRONLY, 0600);
        if(fileDescriptor == -1){
            fprintf(stderr,"File didn't open\n");
            return;
        }
        dup2(fileDescriptor, STDOUT_FILENO);
        close(fileDescriptor);
    }
    if (cmd->file_in != NULL)//read
    {
        fileDescriptor = open(cmd->file_in, O_RDONLY, 0600);
        if(fileDescriptor == -1){
            fprintf(stderr,"File didn't open\n");
            return;
        }
        dup2(fileDescriptor, STDIN_FILENO);
        close(fileDescriptor);
    }
}

void execute(command_list* head, process_list* process){
    if(head == NULL){
        return;
    }
    command* cmd = head->cmd;
    pid_t pid;
    if((pid = fork()) == -1){
        perror("Child process could not be created\n");
        return;
    }
    if (head->background == 0 & pid != 0){
        signal (SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO,pid);
        signal (SIGTTOU, SIG_DFL);
    }
    if(pid == 0){
        setpgid(getpid(),getpid());
        signal(SIGTTIN,SIG_DFL);
        signal(SIGTTOU,SIG_DFL);
        signal(SIGTSTP,SIG_DFL);
        signal(SIGCHLD,SIG_DFL);
        file_io(cmd);
        if (execvp(cmd->tokens[0],cmd->tokens)==-1){
            perror( COLOR_YELLOW "Command not found" COLOR_ALL_STRING);
            _exit(1);
        }
        _exit(0);
    }
    if(pid != 0){
       process_list_insert(process,1,pid);
    }
    if (head->background == 0){
/*
        signal (SIGTTOU, SIG_IGN);
        if(tcsetpgrp(STDIN_FILENO,pid) == -1){
            perror("tcsetpgrp 1");
            return;
        }
        signal (SIGTTOU, SIG_IGN);
*/
        int status = 0;
        int p = waitpid(pid,&status,WUNTRACED | WEXITED);
        if(WIFSTOPPED(status) ){
            fprintf(stderr,"\nProcess %d received a SIGTSTP signal\n",p);
        }else {
            process_list_remove(process,pid);
        }
        if(signal (SIGTTOU, SIG_IGN) == SIG_ERR){
            perror("signal ");
            _exit(-1);
        }
        if(tcsetpgrp(STDIN_FILENO,getpgrp()) == -1){
            perror("tcsetpgrp");
            return;
        }
        if(signal (SIGTTOU, SIG_DFL) == SIG_ERR){
            perror("signal ");
            return;
        }
    }else{
        fprintf(stderr,"Process created with PID: %d\n",pid);
    }
}

int execute_pipe( command_list* head, process_list* process){
    if(head == NULL || process == NULL){
        return -1;
    }
    size_t pnum = head->sizelist * 2;
    int* pipes = (int*) malloc(sizeof(int)*head->sizelist*2);
    if(pipes == NULL){
        fprintf(stderr, "memory wasn't allocated");
        return 1;
    }
    for(int i = 0; i < pnum;i+=2){
        if(pipe(pipes+i) == -1){
            perror("pipe error");
            free(pipes);
            return -1;
        }
    }
    command *temp = head->cmd;
    pid_t     pid;
    pid_t     pgid;
    size_t commands = head->sizelist;
    for(size_t i = 0; i < commands; i++){
        if( temp->next == NULL || temp->next->numtokens == 0){
            process_list_insert(process,commands,pgid);
        }
        if((pid = fork()) == -1){
            perror("Child process could not be created\n");
            free(pipes);
            return -1;
        }
        if(i == 0 && pid != 0){
            pgid = pid;
            if(head->background == 0){
                //perror("dfg");
                signal (SIGTTOU, SIG_IGN);
                tcsetpgrp(STDIN_FILENO,pgid);
                signal (SIGTTOU, SIG_DFL);
            }
        }
        if(pid == 0){
            if(i == 0){
                setpgid(getpid(),getpid());
            }
            else{
                setpgid(getpid(),pgid);
            }
            signal(SIGTTIN,SIG_DFL);
            signal(SIGTTOU,SIG_DFL);
            signal(SIGTSTP,SIG_DFL);
            signal(SIGCHLD,SIG_DFL);
            //perror("dfg");
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
            _exit(0);
        }
        temp=temp->next;
    }

    for(int i=0;i < pnum-2;i++){
        close(pipes[i]);							//close all pipes in parent
    }
    free(pipes);
    if(head->background == 0){
/*
        signal (SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO,pgid);
        signal (SIGTTOU, SIG_DFL);
*/
        waiting(process, head->sizelist, pgid);
        signal (SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO,getpgrp());
        signal (SIGTTOU, SIG_DFL);
    }else{
        fprintf(stderr,"Process created with PID: %d\n",pgid);
    }
    //perror("sdfsdf");
    return 0;
}

int execute_launch_stopped_prog(process_list* process, char* pid_, int back){
    if(pid_ == NULL){
        return EXIT_FAILURE;
    }
    pid_t pid = (pid_t)atoi(pid_);
    if(pid == 0){
        perror("Call atoi");
        return EXIT_FAILURE;
    }
    bp* pgid = process_list_get_pgid(process, pid);
    if(pgid == NULL){
        return EXIT_FAILURE;
    }
    if(back == 0){
        signal (SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO,pid);
        signal (SIGTTOU, SIG_DFL);
    }
    if(killpg(pid,SIGCONT) == -1){
        return EXIT_FAILURE;
    }
    if(back == 0){
/*
        signal (SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO,pid);
        signal (SIGTTOU, SIG_DFL);
*/
        waiting(process,pgid->depth,pid);
        signal (SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO,getpgrp());
        signal (SIGTTOU, SIG_DFL);
    }else{
        fprintf(stderr,"Process created with PID: %d\n",pid);
    }
    return EXIT_SUCCESS;
}
