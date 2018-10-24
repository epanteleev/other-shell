#ifndef PROCESS_LIST_H
#define PROCESS_LIST_H
#include <stdio.h>
#include "common.h"

#define LENGTH_COMMAND 64

typedef struct bp{
    size_t   depth;
    pid_t   pgid;
    struct bp*  next;
}bp;

typedef struct process_list
{
    bp*  prs;
    size_t sizelist;
}process_list;

int process_list_insert(process_list*head,size_t pid,pid_t pgid);

bp * process_list_remove(process_list* head, pid_t data);

bp* process_list_get_pgid(process_list* head, pid_t pgid);

void process_list_destroy(process_list* head);

void process_list_init(process_list* head);

void process_list_print(process_list* head);
#endif
