#ifndef EXECUTE_H
#define EXECUTE_H
#include "common.h"
#include "process_list.h"
#include "signal_handler.h"
#include "parser.h"

void execute(command_list* head, process_list* process);

int piped_execute( command_list* head, process_list* process);

int launch_stopped_prog(process_list* process, char* pid_, int back);

#endif
