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
    struct command* next;
}command;

typedef struct command_list
{
    command* cmd;
    size_t sizelist;
    unsigned char background;
}command_list;

///
/// \brief initial_parser
/// \param head
void initial_parser(command_list* head);

///
/// \brief get_next_tokens
/// \param line входная строка
/// \param head список, куда будут записаны аргументы
/// \return указатель на не обработанную строку
///
char* get_next_tokens(char* line, command_list* head);

///
/// \brief reset_parser очищает список.
void reset_parser(command_list* head);

#endif
