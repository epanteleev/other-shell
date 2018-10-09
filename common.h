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
#include <stdbool.h>
#include <sys/prctl.h>
pid_t CURRENT_PID;
int NO_PRINT;

#define LIMIT 255
#define MAXLINE 1024
#define FILE_FORMAT ".sh"
#define EXIT_COMMAND "exit"

#define OUTPUT 1
#define INPUT 0

typedef struct command_
{
    char* tokens[LIMIT];
    size_t numtokens;
    char* file_in;
    char* file_out;
    unsigned char background;
}command;

#endif //COMMON_H
