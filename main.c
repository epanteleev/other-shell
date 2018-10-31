#include <assert.h>
#include <sys/wait.h>

#include "parser.h"
#include "signal_handler.h"
#include "common.h"
#include "process_list.h"

int shell_init(){
   // int interactive = isatty(STDERR_FILENO);
    int shell_pgid = getpid();
    signal (SIGINT, SIG_IGN);
    signal (SIGTSTP, SIG_IGN);
    signal (SIGQUIT, SIG_IGN);
    signal (SIGTTIN, SIG_IGN);
    signal (SIGTTOU, SIG_IGN);
    if(signal(SIGCHLD,signal_handler_child)==SIG_ERR){
        perror("signal not caught!!");
        return EXIT_FAILURE;
    }

    shell_pgid = getpid();
    if(setpgid(shell_pgid,shell_pgid) == -1){
        perror("Can't make shell a member of it's own process group");
        return EXIT_FAILURE;
    }
    tcsetpgrp(STDERR_FILENO,shell_pgid);
    return EXIT_SUCCESS;
}

int change_directory(char** args){
    if (args[1] == NULL){
        if(chdir(getenv("HOME"))){
            perror("chdir");
        }
        return EXIT_SUCCESS;
    }
    if (chdir(args[1]) == -1){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void shell_prompt(){
    char hostname[1024];
    char current_directory[1024];
    gethostname(hostname, sizeof(hostname));

    fprintf(stderr,"\x1b[1;31m%s@%s\x1b[0m$ \x1b[1;35m%s\x1b[0m$ ", getenv("LOGNAME"), hostname, getcwd(current_directory, 1024));
}

void file_io(command* cmd)
{
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

int waiting(process_list* process,size_t num_process, pid_t pgid){
    for(size_t i = 0;i < num_process;i++){
        int status = 0;
        int p = waitpid(-pgid,&status,WUNTRACED);
        if(WIFSTOPPED(status) ){
            if(p == pgid)
            fprintf(stderr,"\nProcess %d received a SIGTSTP signal\n",p);
        }else {
            process_list_remove(process,p);
        }
    }
    return 0;
}

void launch_prog(command_list* head, process_list* process){
    if(head == NULL){
        return;
    }
    command* cmd = head->cmd;
    int pid = -2;
    if((pid = fork()) == -1)
    {
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
        setpgid(getpid(),getpid());
        file_io(cmd);
        if (execvp(cmd->tokens[0],cmd->tokens)==-1){
            perror("\x1b[3;31mCommand not found\x1b[3m");
            _exit(1);
        }
    }
    if(pid != 0){
       process_list_insert(process,1,pid);
    }
    if (head->background == 0){
        tcsetpgrp(STDERR_FILENO,pid);
        int status = 0;
        int p = waitpid(pid,&status,WUNTRACED);
        if(WIFSTOPPED(status) ){
            fprintf(stderr,"\nProcess %d received a SIGTSTP signal\n",p);
        }else {
            process_list_remove(process,pid);
        }
        tcsetpgrp(STDERR_FILENO,getpid());
    }else{
        fprintf(stderr,"Process created with PID: %d\n",pid);
    }
    fflush(stdout);
}

int piped_execute( command_list* head, process_list* process){
    if(head == NULL || process == NULL){
        return -1;
    }
    int pnum = head->sizelist * 2;
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
    sigset_t set;
    lock_sigchld(&set);
    command *temp = head->cmd;
    int     pid = -2;
    int     pgid = -2;
    size_t commands = head->sizelist;
    for(size_t i = 0; i < commands; i++)
    {
        pid = fork();
        if( temp->next == NULL || temp->next->numtokens == 0){
            process_list_insert(process,commands,pgid);
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
            for(int k = 0;k < pnum - 2;k++){
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

    for(int i=0;i < pnum-2;i++){
        close(pipes[i]);							//close all pipes in parent
    }
    free(pipes);
        //process_list_print(process);
    if(head->background == 0){
        tcsetpgrp(STDERR_FILENO,pgid);
        waiting(process, head->sizelist, pgid);
        tcsetpgrp(STDERR_FILENO,getpid());
    }else{
        fprintf(stderr,"Process created with PID: %d\n",pgid);
    }
    unlock_sigchld(&set);
    return 0;
}

int launch_stopped_prog(process_list* process, char* pid_, int back)
{
    if(pid_ == NULL){
        return EXIT_FAILURE;
    }
    int pid = atoi(pid_);
    if(pid == 0){
        perror("Call atoi");
        return EXIT_FAILURE;
    }
    sigset_t set;
    lock_sigchld(&set);
    bp* pgid = process_list_get_pgid(process, pid);
    if(pgid == NULL){
        fprintf(stderr,"dfsfsf\n");
        return EXIT_FAILURE;
    }
    if(killpg(pid,SIGCONT) == -1){
        return EXIT_FAILURE;
    }
    if(back == 0){
        tcsetpgrp(STDERR_FILENO,pid);
        waiting(process,pgid->depth,pid);
        tcsetpgrp(STDERR_FILENO,getpid());
    }else{
        fprintf(stderr,"Process created with PID: %d\n",pid);
    }
    unlock_sigchld(&set);
    return EXIT_SUCCESS;
}

int command_handler(command_list* head, process_list* process)
{
    if(head->sizelist == 0 ){
        return -1;
    }
    command* cmd = head->cmd;
    if(strcmp(cmd->tokens[0],EXIT_COMMAND) == 0){
        return EXIT_SUCCESS;
    }
    else if (strcmp(cmd->tokens[0],CD_COMMAND) == 0)
    {
        if(change_directory(cmd->tokens)!= 0){
             fprintf(stderr,"%s: no such directory\n", cmd->tokens[1]);
        }
        return -1;
    }
    else if (strcmp(cmd->tokens[0],LAUNCH_STOP_PRCS) == 0)
    {
        if(launch_stopped_prog(process, cmd->tokens[1], 0 )){
            fprintf(stderr, "Process is not started\n");
        }
        return -1;
    }
    else if (strcmp(cmd->tokens[0],"bg") == 0)
    {
        if(launch_stopped_prog(process, cmd->tokens[1], 1)){
            fprintf(stderr, "Process is not started\n");
        }
        return -1;
    }
    else if (strcmp(cmd->tokens[0],JOBS) == 0)
    {
        process_list_print(process);
        return -1;
    }
    else if (cmd->next != NULL && cmd->next->numtokens != 0)
    {
        piped_execute(head,process);
    }else{
        launch_prog(head,process);
    }
    return -1;
}

int get_line(char *line, size_t __n){
    if(fgets(line, __n, stdin) == 0){
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[], char ** envp){
    if(shell_init()){

        return EXIT_FAILURE;
    }
    command_list cmd;
    process_list process;
    initial_parser(&cmd);
    process_list_init(&process);
    char    line[MAXLINE];
    int     end = 1;
    while (end){
        shell_prompt();
        memset(line, 0, MAXLINE);
        get_line(line, MAXLINE);
        char* current_pos = line;
        while(current_pos != NULL){
            current_pos = get_next_tokens(current_pos,&cmd);
            if(!command_handler(&cmd, &process)){
                end = 0;
                break;
            }
        }
    }
    reset_parser(&cmd);
    process_list_destroy(&process);
    return EXIT_SUCCESS;
}
