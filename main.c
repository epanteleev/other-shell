#include <assert.h>
#include <sys/wait.h>
#include <errno.h>

#include "parser.h"
#include "common.h"
#include "process_list.h"
#include "execute.h"

void child(){
    int status;
    pid_t pid;
    //perror("sfsfds");
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {}
}

int shell_init(){
    int interactive = isatty(STDIN_FILENO);
    pid_t shell_pgid;
    if(interactive){
        while (tcgetpgrp(STDIN_FILENO) !=(shell_pgid = getpgrp())) {
            kill(-shell_pgid,SIGTTIN);
        }
    }
    else{
        fprintf(stderr, "Could not make the shell interactive.\n");
        return EXIT_FAILURE;
    }
    shell_pgid = getpid();
    if(getsid(shell_pgid) == shell_pgid){
        fprintf(stderr,"process is lider session\n");
        return EXIT_FAILURE;
    }
    signal (SIGINT, SIG_IGN);
    signal (SIGTSTP, SIG_IGN);
    signal (SIGQUIT, SIG_IGN);
    signal (SIGCHLD, SIG_DFL);
    signal (SIGTTIN, SIG_DFL);
/*
    if(setpgid(getpid(),getpid()) == -1){
        perror("Can't make shell a member of it's own process group");
        return EXIT_FAILURE;
    }
*/
    signal (SIGTTOU, SIG_IGN);
    if(tcsetpgrp(STDIN_FILENO,getpgrp()) == -1){
        perror("tcsetpgrp");
        return EXIT_FAILURE;
    }
    signal (SIGTTOU, SIG_DFL);
    chdir(getenv("HOME"));
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
static int form_prompt(char* prompt) {
    //char current_directory[512];
    //char hostname[512];
    memset(prompt,0,MAXLINE);
   // gethostname(hostname, sizeof(hostname));

    if (snprintf(prompt, MAXLINE,COLOR_RED "%s" COLOR_NONE "~$ ", getenv("LOGNAME")) <= 0) {
        fprintf(stderr, "Cannot form prompt.");
        return 0;
    }
    return 1;
}

void shell_prompt(){
    char hostname[1024];
    form_prompt(hostname);
    fprintf(stderr,"%s",hostname);
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
        if(change_directory(cmd->tokens) != 0){
             fprintf(stderr,"%s: no such directory\n", cmd->tokens[1]);
        }
        return -1;
    }
    else if (strcmp(cmd->tokens[0],FOREGROUND) == 0)
    {
        if(execute_launch_stopped_prog(process, cmd->tokens[1], 0 )){
            fprintf(stderr, "Process is not started\n");
        }
        return -1;
    }
    else if (strcmp(cmd->tokens[0],BACKGROUND) == 0)
    {
        if(execute_launch_stopped_prog(process, cmd->tokens[1], 1)){
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
        execute_pipe(head,process);
    }else{
        execute(head,process);
    }
    return -1;
}

char* get_line(char *line, size_t __n){
    char* i = fgets(line, __n, stdin);
    return i;
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
    char    prm[MAXLINE*2];
    int     end = 1;
    while (end){
        memset(line, 0, MAXLINE);
        shell_prompt();
        char * res = get_line(line, MAXLINE);
        if(res == NULL){
            fprintf(stderr,"\n");
            break;
        }
        char* current_pos = res;
        while(current_pos != NULL){
            current_pos = get_next_tokens(current_pos,&cmd);
            if(!command_handler(&cmd, &process)){
                end = 0;
                break;
            }
        }
        child();
    }
    reset_parser(&cmd);
    process_list_destroy(&process);
    return EXIT_SUCCESS;
}
