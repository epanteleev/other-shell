#include <assert.h>
#include<sys/wait.h>
#include "parser.h"
#include "signal_handler.h"
#include "common.h"

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

int shell_init()
{
    shell_terminal = STDERR_FILENO;
    int interactive = isatty(shell_terminal);
    if(interactive){
        while(tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp())){
            kill(- shell_pgid,SIGTTIN);
        }
    }
    signal (SIGINT, SIG_IGN);
    signal (SIGTSTP, SIG_IGN);
    signal (SIGQUIT, SIG_IGN);
    signal (SIGTTIN, SIG_IGN);
    signal (SIGTTOU, SIG_IGN);
    if(signal(SIGINT,signal_handler_int)==SIG_ERR){
         perror("Signal not caught!!");
    }
    if(signal(SIGCHLD,signal_handler_child)==SIG_ERR){
        perror("signal not caught!!");
    }

    shell_pgid = getpid();
    if(setpgid(shell_pgid,shell_pgid)<0)
    {
        perror("Can't make shell a member of it's own process group");
        _exit(1);
    }
    tcsetpgrp(shell_terminal,shell_pgid);
    setenv("SHELL", getcwd(current_directory, 1024), 1);
    return EXIT_SUCCESS;
}

void file_io(command* cmd)
{
    if(cmd == NULL){
        fprintf(stderr, "incorrect argument\n");
    }
    int fileDescriptor; // between 0 and 19, describing the output or input file
    if (cmd->file_out != NULL)//write
    {
        fileDescriptor = open(cmd->file_out, O_CREAT | O_TRUNC | O_WRONLY, 0600);
        if(fileDescriptor == -1){
            fprintf(stderr,"File didn't open\n");
            _exit(1);
        }
        dup2(fileDescriptor, STDOUT_FILENO);
        close(fileDescriptor);
    }
    if (cmd->file_in != NULL)//read
    {
        fileDescriptor = open(cmd->file_in, O_RDONLY, 0600);
        if(fileDescriptor == -1){
            fprintf(stderr,"File didn't open\n");
            _exit(1);
        }
        dup2(fileDescriptor, STDIN_FILENO);
        close(fileDescriptor);
    }
}

void launch_prog(command* cmd)
{
    if((CURRENT_PID = fork()) == -1)
    {
        perror("Child process could not be created\n");
        return;
    }
    if(CURRENT_PID==0)
    {
        //(PR_SET_PDEATHSIG, SIGHUP);		//kill the child when parent dies.(prevents formation of zombie process)
        setpgid(getpid(),getpid());
        if(cmd->background == 0){
            tcsetpgrp(shell_terminal,getpid());
        }
        signal (SIGINT, SIG_DFL);
        signal (SIGQUIT, SIG_DFL);
        signal (SIGTSTP, SIG_DFL);
        signal (SIGTTIN, SIG_DFL);
        signal (SIGTTOU, SIG_DFL);
        signal (SIGCHLD, SIG_DFL);
        file_io(cmd);
        if (execvp(cmd->tokens[0],cmd->tokens)==-1)
        {
            perror("\x1b[3;31mCommand not found\x1b[3m");
            _exit(1);
        }
        _exit(0);
    }

    if (cmd->background == 0){
        tcsetpgrp(shell_terminal,CURRENT_PID);
        int status;
        waitpid(CURRENT_PID,&status,WUNTRACED);
        if(WIFSTOPPED(status)){
            fprintf(stderr,"\nProcess %d received a SIGTSTP signal\n",CURRENT_PID);
        }
        tcsetpgrp(shell_terminal,shell_pgid);
    }else{
        fprintf(stderr,"Process created with PID: %d\n",CURRENT_PID);
    }
    fflush(stdout);
}

int piped_execute( command head[])
{
    int pnum = 30;
    int p=0,i,j=pnum-1,pgid,pipes[2*(pnum-1)],comc=0;
    for(i=0;j--;i+=2)
    {
        if((pipe(pipes+i))<0)
        {
            perror("pipe error");
            return -1;
        }
    }
    //signal (SIGCHLD, SIG_IGN);
    command *temp=head;
    while(temp!=NULL && temp->numtokens != 0)
    {
        int pid=fork();
        if( temp->next == NULL || temp->next->numtokens == 0)
            //insert_process(temp->tokens[0],pid,pgid);
        if(comc==0&&pid!=0)
            pgid=pid;
        if(pid!=0)
            setpgid(pid,pgid);
        if(pid==0)
        {
            signal (SIGINT, SIG_DFL);
            signal (SIGQUIT, SIG_DFL);
            signal (SIGTSTP, SIG_DFL);
            signal (SIGTTIN, SIG_DFL);
            signal (SIGTTOU, SIG_DFL);
            signal (SIGCHLD, SIG_DFL);

            if(temp->next!=NULL && temp->next->numtokens != 0)
            {
                if((dup2(pipes[2*comc+1],1))<0)
                {
                    perror("dup2 error");
                }
            }
            file_io(temp);
            if(comc!=0)
            {
                if((dup2(pipes[2*(comc-1)],0))<0)
                {
                    perror("dup2 error");
                }
            }
            for(i=0;i<2*(pnum-1);i++)
                close(pipes[i]);
            if((execvp(temp->tokens[0],temp->tokens))<0){
                perror("Cannot ececute");
                _exit(-1);
            }
            _exit(0);
        }
        else if(pid<0)
        {
            perror("Could not fork child");
            return -1;
        }
        temp=temp->next;
        if(temp!=NULL && temp->numtokens != 0)
        {
            comc++;
        }
    }

    for(i=0;i<2*(pnum-1);i++)
    {
        close(pipes[i]);							//close all pipes in parent
    }
    if(head->background==0)
    {
        tcsetpgrp(shell_terminal,pgid);
        for(i=0;i<comc;i++)
        {
            int status;
            //int ppp;
            //ppp=waitpid(-1,&status,WUNTRACED);
            wait(&status);
        }
        tcsetpgrp(shell_terminal,getpid());
    }
    //signal (SIGCHLD, SIG_DFL);
    return p;
}

int launch_stopped_prog(char* pid_)
{
    if(pid_ == NULL)
    {
        return EXIT_FAILURE;
    }
    int i = atoi(pid_);
    if(i == 0)
    {
        perror("Call atoi");
        return EXIT_FAILURE;
    }
    CURRENT_PID = i;
    if(kill(i,SIGCONT) == -1)
    {
        return EXIT_FAILURE;
    }
    tcsetpgrp(shell_terminal,CURRENT_PID);
    int status;
    waitpid(i,&status,WUNTRACED);
    if(WIFSTOPPED(status)){
        fprintf(stderr,"\nProcess %d received a SIGTSTP signal\n",CURRENT_PID);
    }
    tcsetpgrp(shell_terminal,shell_pgid);
    return EXIT_SUCCESS;
}

int command_handler(command* cmd)
{
    if(cmd == NULL || cmd->numtokens == 0 ){
        return -1;
    }
    if(strcmp(cmd->tokens[0],EXIT_COMMAND) == 0){
        return EXIT_SUCCESS;
    }
    else if (strcmp(cmd->tokens[0],"cd") == 0)
    {
        if(change_directory(cmd->tokens)!= 0){
             fprintf(stderr,"%s: no such directory\n", cmd->tokens[1]);
        }
        return -1;
    }
    else if (strcmp(cmd->tokens[0],"fg") == 0)
    {
        if(launch_stopped_prog(cmd->tokens[1])){
            fprintf(stderr, "Process is not started\n");
        }
        return -1;
    }
    else if (cmd->next != NULL && cmd->next->numtokens != 0)
    {
        piped_execute(cmd);
    }else{
        launch_prog(cmd);
    }
    return -1;
}



int get_line(char *line, size_t __n)
{
    if(fgets(line, __n, stdin) == 0){
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[], char ** envp)
{
  //  setbuf(stdout,NULL);
    if(shell_init() != 0){
        return -1;
    }
    command cmd;
    initial_parser(&cmd);
    char line[MAXLINE];
    CURRENT_PID = -10;
    while (1){
        shell_prompt();
        memset(line, 0, MAXLINE);
        get_line(line, MAXLINE);
        char* current_pos = line;
        while(current_pos != NULL){
            current_pos = get_next_tokens(current_pos,&cmd);
            if(!command_handler(&cmd)){
                return EXIT_SUCCESS;
            }
        }
    }

    reset_parser(&cmd);
    return EXIT_SUCCESS;
}
