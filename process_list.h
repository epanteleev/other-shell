#ifndef PROCESS_LIST_H
#define PROCESS_LIST_H
#include <stdio.h>
#include "common.h"

#define LENGTH_COMMAND 64

typedef struct jobs{
    size_t   depth; //количество процессов в группе
    pid_t   pgid;
    size_t id;
    struct jobs*  next;
}jobs;

typedef struct process_list{
    jobs*  prs;
    size_t sizelist;
}process_list;

size_t process_list_insert(process_list*head,size_t pid,pid_t pgid);

jobs* process_list_remove(process_list* head, pid_t data);

jobs* process_list_get(process_list* head, size_t id);

void process_list_destroy(process_list* head);

void process_list_init(process_list* head);

void process_list_print(process_list* head);
#endif
