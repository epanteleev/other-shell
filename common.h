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
#define CD_COMMAND "cd"
#define LAUNCH_STOP_PRCS "fg"
#define JOBS "jobs"

#endif //COMMON_H
