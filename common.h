#ifndef COMMON_H
#define COMMON_H
//#define __GNUG__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

#define LIMIT 255
#define MAXLINE 1024
#define FILE_FORMAT ".sh"
#define EXIT_COMMAND "exit"

#define OUTPUT 1
#define INPUT 0

pid_t CURRENT_PID;


char current_directory[MAXLINE];
pid_t shell_pgid;
int shell_terminal,redirection;

#endif //COMMON_H
