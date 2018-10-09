#include "signal_handler.h"
#include "common.h"
#include <assert.h>

static char current_directory[MAXLINE];
pid_t shell_pgid;			//shell's process group id
int shell_terminal,back,redirection;

void shell_prompt()
{
    char hostname[1024];
    gethostname(hostname, sizeof(hostname));
    printf("\x1b[1;31m%s@%s\x1b[0m$ \x1b[1;35m%s\x1b[0m$ ", getenv("LOGNAME"), hostname, getcwd(current_directory, 1024));
}

int change_directory(char** args)
{
    if (args[1] == NULL){
        if(chdir(getenv("HOME"))){
            perror("chdir");
        }
        return EXIT_SUCCESS;
    }
    if (chdir(args[1]) == -1)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int shell_init()
{
    shell_terminal = STDIN_FILENO;back=0;
    int interactive = isatty(shell_terminal);
    if(interactive)
    {
        while(tcgetpgrp(shell_terminal) != (shell_pgid=getpgrp()))
            kill(- shell_pgid,SIGTTIN);
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

    shell_pgid=getpid();
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
        if(fileDescriptor == -1)
        {
            fprintf(stderr,"File didn't open\n");
            _exit(1);
        }
        dup2(fileDescriptor, STDOUT_FILENO);
        close(fileDescriptor);
    }
    if (cmd->file_in != NULL)//read
    {
        fileDescriptor = open(cmd->file_in, O_RDONLY, 0600);
        if(fileDescriptor == -1)
        {
            fprintf(stderr,"File didn't open\n");
            kill(getpid(),SIGTERM);
        }
        dup2(fileDescriptor, STDIN_FILENO);
        close(fileDescriptor);
    };
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
        prctl(PR_SET_PDEATHSIG, SIGHUP);		//kill the child when parent dies.(prevents formation of zombie process)
        setpgid(getpid(),getpid());
        if(cmd->background == 0)
            tcsetpgrp(shell_terminal,getpid());
        /*make the signals for the child as default*/
        signal (SIGINT, SIG_DFL);
        signal (SIGQUIT, SIG_DFL);
        signal (SIGTSTP, SIG_DFL);
        signal (SIGTTIN, SIG_DFL);
        signal (SIGTTOU, SIG_DFL);
        signal (SIGCHLD, SIG_DFL);
        file_io(cmd);
        //setenv("parent",getcwd(currentDirectory, 1024),1);
        if (execvp(cmd->tokens[0],cmd->tokens)==-1)
        {
            perror("\x1b[3;31mCommand not found\x1b[3m");
            _exit(1);
            //raise(SIGTERM);
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
            //remove_process(pid);
        tcsetpgrp(shell_terminal,shell_pgid);
    }else{
        fprintf(stderr,"Process created with PID: %d\n",CURRENT_PID);
    }
    fflush(stdout);
}

void command_clear(command* cmd){
    cmd->file_in = NULL;
    cmd->file_out = NULL;
    cmd->numtokens = 0;
}
void pipe_handler(command* cmd)
{
    if(cmd == NULL)
    {
        abort();
    }
    int filedes[2]; // pos. 0 output, pos. 1 input of the pipe
    int filedes2[2];
    int end = 0;

    // Переменные, используемые для разных циклов
    int i = 0;

    int l = 0;
    // вычисляем количество команд
    int size_commands = 0;
    while (cmd->tokens[l] != NULL)
    {
        if (strcmp(cmd->tokens[l],"|") == 0){
            size_commands++;
        }
        l++;
    }
    size_commands++;


    // Основной цикл этого метода. Для каждой команды между '|',
    // будут сконфигурированы каналы, а стандартный ввод и или вывод будут
    // заменены
    int j = 0;
    int pgid = 0;
    while (cmd->tokens[j] != NULL && end != 1)
    {
        int k = 0;
        //используем вспомогательный массив указателей для хранения команды
        //, который будет выполняться на каждой итерации
        command current;
        command_clear(&current);
        while (strcmp(cmd->tokens[j],"|") != 0)
        {
            if (strcmp(cmd->tokens[j],"<") == 0)
            {
                if (cmd->tokens[j+1] == NULL ){
                    fprintf(stderr,"Not enough input arguments\n");
                    return;
                }
                current.file_in = cmd->tokens[j+1];
                j++;

            }
            else if (strcmp(cmd->tokens[j],">") == 0)
            {
                if (cmd->tokens[j+1] == NULL){
                    fprintf(stderr,"Not enough input arguments: s.%d in file %s\n",__LINE__, __FILE__);
                    return;
                }

                current.file_out = cmd->tokens[j+1];
                j++;
                //file_io(cmd->tokens,NULL,cmd->tokens[i+1],0);
                //return -1;
            }else{
                current.tokens[k] = cmd->tokens[j];
            }

            j++;
            if (cmd->tokens[j] == NULL)
            {
                end = 1;
                k++;
                break;
            }
            k++;
        }
        current.tokens[k] = NULL;
        j++;


        // В зависимости от того, находимся ли мы на итерации или нет, мы
        // будет устанавливать разные дескрипторы для входов и
        // вывод. Таким образом, между двумя
        // итерации, позволяющие подключать входы и выходы
        // две разные команды.
        if (i % 2 != 0){
            if(pipe(filedes)<0){
               perror("pipe");
               _exit(-1);
            }
        }else{
            if(pipe(filedes2)<0){
                perror("pipe");
                _exit(-1);
            }
        }

        CURRENT_PID = fork();
        if(i == 0 && CURRENT_PID!=0){
            pgid=CURRENT_PID;
        }
                                         // pgid is storing the proces id of child
        if(CURRENT_PID!=0)
            setpgid(CURRENT_PID,pgid);

        if(CURRENT_PID == -1)
        {
            if (i != size_commands - 1)
            {
                if (i % 2 != 0){
                    close(filedes[1]);
                }else{
                    close(filedes2[1]);
                }
            }
            fprintf(stderr,"Child process could not be created\n");
            break;
        }
        if(CURRENT_PID==0)
        {

            signal (SIGINT, SIG_DFL);
            signal (SIGQUIT, SIG_DFL);
            signal (SIGTSTP, SIG_DFL);
            signal (SIGTTIN, SIG_DFL);
            signal (SIGTTOU, SIG_DFL);
            signal (SIGCHLD, SIG_DFL);
            if (i == 0){
                dup2(filedes2[1], STDOUT_FILENO);
            }
            else if (i == size_commands - 1)
            {
                if (size_commands % 2 != 0){
                    dup2(filedes[0],STDIN_FILENO);
                }else{
                    dup2(filedes2[0],STDIN_FILENO);
                }
            }
            else
            {
                //printf("HERE656bb\n");
                if (i % 2 != 0){
                    dup2(filedes2[0],STDIN_FILENO);
                    dup2(filedes[1],STDOUT_FILENO);
                }else{
                    dup2(filedes[0],STDIN_FILENO);
                    dup2(filedes2[1],STDOUT_FILENO);
                }
            }
            file_io(&current);
            if (execvp(current.tokens[0],current.tokens) == -1)
            {
                perror("Call execvp");
                _exit(1);
            }
            _exit(0);
        }

        if (i == 0)
        {
            close(filedes2[1]);
        }
        else if (i == size_commands - 1)
        {
            if (size_commands % 2 != 0){
                close(filedes[0]);
            } else{
                close(filedes2[0]);
            }
        }
        else
        {
            if (i % 2 != 0){
                close(filedes2[0]);
                close(filedes[1]);
            }else{
                close(filedes[0]);
                close(filedes2[1]);
            }
        }

        i++;
    }
    tcsetpgrp(shell_terminal,pgid);						//set the terminal control to child
    for(int i=0;i<size_commands;i++)
    {
        int status;
        waitpid(-pgid,&status,WUNTRACED);
        if(WIFSTOPPED(status)){
            fprintf(stderr,"Process created with PID: %d\n",CURRENT_PID);
        }
        //else
        //	killpg(pgid,SIGSTOP);
    }
    tcsetpgrp(shell_terminal,shell_pgid);

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
    //setpgid(getpid(),CURRENT_PID);
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
    if(cmd == NULL || cmd->tokens[0] == NULL ){
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
    for (int i = 0; i < cmd->numtokens; i++)
    {
        if (strcmp(cmd->tokens[i],"|") == 0){
            pipe_handler(cmd);
            return -1;
        }
    }
    for (int i = 0; i < cmd->numtokens; i++)
    {
        if (strcmp(cmd->tokens[i],"<") == 0)
        {
            if (cmd->tokens[i+1] == NULL ){
                fprintf(stderr,"Not enough input arguments\n");
                return -1;
            }
            cmd->file_in = cmd->tokens[i+1];
            cmd->tokens[i] = NULL;
            cmd->tokens[i+1] = NULL;
            cmd->numtokens = cmd->numtokens - 2;
        }
        else if (strcmp(cmd->tokens[i],">") == 0)
        {
            if (cmd->tokens[i+1] == NULL){
                fprintf(stderr,"Not enough input arguments: s.%d in file %s\n",__LINE__, __FILE__);
                return -1;
            }

            cmd->file_out = cmd->tokens[i+1];
            cmd->tokens[i] = NULL;
            cmd->tokens[i + 1] = NULL;
            cmd->numtokens = cmd->numtokens - 2;
        }
    }

    launch_prog(cmd);

    return -1;
}

char* get_next_tokens(char* line, command* cmd)
{
    if(line == NULL){
        return NULL;
    }
    char* buff_line = line;
    char* ret;
    if((ret = strchr(line, ';')) != NULL)
    {
        buff_line = strtok(line, ";");
        ret = strtok(NULL, "");
    }
    if ((cmd->tokens[0] = strtok(buff_line, " \n\t")) == NULL)
    {
        return 0;
    }
    int numTokens = 1;
    while ((cmd->tokens[numTokens] = strtok(NULL, " \n\t")) != NULL)
    {
        numTokens++;
        if(numTokens == LIMIT)
        {
            fprintf(stderr,"Word limit is exceeded\n");
            break;
        }
    }
    cmd->numtokens = numTokens;
    if(strcmp(cmd->tokens[numTokens-1],"&") == 0){
        cmd->background = 1;
        cmd->tokens[numTokens-1] = NULL;
        cmd->numtokens--;
    }else {
        cmd->background = 0;
    }
    cmd->file_in = NULL;
    cmd->file_out = NULL;
    return ret;
}

int get_line(char *line, size_t __n)
{
    if(fgets(line, __n, stdin)< 0){
        fprintf(stderr, "err\n");
    }
    return 0;
}

int main(int argc, char *argv[], char ** envp)
{
    if(shell_init() != 0){
        abort();
    }
    command cmd;
    char line[MAXLINE];
    NO_PRINT = 0;
    CURRENT_PID = -10;
    while (1){
        if (NO_PRINT == 0){
            shell_prompt();
        }
        NO_PRINT = 0;
        memset(line, '\0', MAXLINE);
        get_line(line, MAXLINE);
        char* current_pos = line;
        while(current_pos != NULL){
          //  printf("hhfh\n");
            current_pos = get_next_tokens(current_pos,&cmd);
            if(!command_handler(&cmd)){
                return EXIT_SUCCESS;
            }
        }
    }
    return EXIT_SUCCESS;
}
