#ifndef EXECUTE_H
#define EXECUTE_H
#include "common.h"
#include "parser.h"
#include "process_list.h"
#include <sys/wait.h>
void execute(command_list* head, process_list* process);

int execute_pipe( command_list* head, process_list* process);

int execute_launch_stopped_prog(process_list* process, char* pid_, int back);

#endif
