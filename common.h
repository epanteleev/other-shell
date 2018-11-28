#ifndef COMMON_H
#define COMMON_H

#define COLOR_ON
//#define EDITOR_ON

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

#ifdef EDITOR_ON
#include"readline/readline.h"
#include "readline/history.h"
#endif



#define LIMIT 255
#define MAXLINE 1024

#define FILE_FORMAT ".sh"
#define EXIT_COMMAND "exit"
#define CD_COMMAND "cd"
#define FOREGROUND "fg"
#define BACKGROUND "bg"
#define JOBS "jobs"



#ifdef COLOR_ON
#define COLOR_NONE "\033[m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_GREEN "\033[0;32;32m"
#define COLOR_GRAY "\033[1;30m"
#define COLOR_RED "\x1b[1;31m"
#define COLOR_VIOLET "\x1b[1;35m"
#define COLOR_ALL_STRING "\x1b[3m"
#else
#define COLOR_NONE
#define COLOR_RED
#define COLOR_YELLOW
#define COLOR_CYAN
#define COLOR_GREEN
#define COLOR_GRAY
#define COLOR_RED
#define COLOR_VIOLET
#define COLOR_ALL_STRING
#endif

#define VERSION_NUM "0.0.1"



#endif //COMMON_H
