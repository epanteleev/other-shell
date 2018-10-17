#include <stdlib.h>

#include "parser.h"

static void _clear(command* cmd){
    if(cmd == NULL){
        return;
    }
    command* curr = cmd;
    while (curr != NULL) {
        curr->background = 0;
        curr->file_in = NULL;
        curr->file_out = NULL;
        curr->numtokens = 0;
        curr->tokens[LIMIT] = (NULL);
        curr = curr->next;
    }
}

static int _insert_command(command** current)
{
    command* new = NULL;
    (*current)->tokens[(*current)->numtokens] = NULL;
    if((*current)->next == NULL ){
        new=(command *)malloc(sizeof(command));
        if(new == NULL){
            fprintf(stderr,"memory not allocated\n");
            return -1;
        }
        new->next = NULL;
        (*current)->next = new;

    }else{
        new = (*current)->next;
    }
    new->file_out = NULL;
    new->file_in = NULL;
    new->tokens[LIMIT] = (NULL);
    new->background = 0;
    *current = (*current)->next;
    return 0;
}

static int _get_list_tokens(char** tokens, char* line, char** current_pos){
    char* buff_line = line;
    if((*current_pos = strchr(line, ';')) != NULL)
    {
        buff_line = strtok(line, ";");
        *current_pos = strtok(NULL, "");
    }

    if ((tokens[0] = strtok(buff_line, " \n\t")) == NULL)
    {
        return 0;
    }
    int numTokens = 1;
    while ((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL)
    {
        numTokens++;
        if(numTokens == LIMIT)
        {
            fprintf(stderr,"Word limit is exceeded\n");
            break;
        }
    }
    return numTokens;
}

char* get_next_tokens(char* line, command* head)
{
    if(line == NULL || head == NULL){
        return NULL;
    }
    _clear(head);

    char* ret = NULL;
    char* list[LIMIT];
    int numTokens = _get_list_tokens(list,line,&ret);
    if(numTokens == 0){
        return NULL;
    }
    command* curr = head;
    for(size_t i = 0; i < numTokens; i++){
        if(strcmp(list[i],"|") == 0){
            _insert_command(&curr);
        }else if (strcmp(list[i],">") == 0) {
            curr->file_out = list[i+1];
            i++;
        }else if (strcmp(list[i],"<") == 0) {
            curr->file_in = list[i+1];
            i++;
        }else if (strcmp(list[i],"&") == 0) {
            curr->background = 1;
        }else{
            curr->tokens[curr->numtokens] = list[i];
            curr->numtokens++;
        }
    }
    curr->tokens[curr->numtokens] = NULL;
    return ret;
}

void initial_parser(command* head){
    head->next = NULL;
    head->background = 0;
    head->file_in = NULL;
    head->file_out = NULL;
    for(size_t i = 0; i<LIMIT; i++){
        head->tokens[i] = NULL;
    }
}

void reset_parser(command* head){
    command* curr = head->next;
    while (curr != NULL) {
        command* l = curr->next;
        free(curr);
        curr = l;
    }
}

void command_clear(command* cmd){
    command* curr = cmd->next;
    cmd->file_in = NULL;
    cmd->file_out = NULL;
    cmd->numtokens = 0;
    cmd->tokens[LIMIT] = (0);
    while (curr != NULL) {
        command* l = curr->next;
        free(curr);
        curr = l;
    }
}
