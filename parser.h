#ifndef PARSER_H
#define PARSER_H
#include <string.h>

#include "common.h"
#include "stdio.h"

typedef struct command
{
    char* tokens[LIMIT];
    size_t numtokens;
    char* file_in;
    char* file_out;
    unsigned char background;
    struct command* next;
}command;

void initial_parser(command* head);
char* get_next_tokens(char* line, command* head);

void command_clear(command* cmd);

void reset_parser(command* head);
#endif
