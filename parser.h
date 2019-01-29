#ifndef PARSER_H
#define PARSER_H

#include "common.h"

typedef struct command{
    char* tokens[LIMIT];
    size_t numtokens;
    char* file_in;
    char* file_out;
    struct command* next;
}command;

typedef struct command_list{
    command* cmd;
    size_t sizelist;
    unsigned char background;
}command_list;

void initial_parser(command_list* head);

char* get_next_tokens(char* line, command_list* head);

void reset_parser(command_list* head);

#endif
