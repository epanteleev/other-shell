#include "process_list.h"

static size_t current_id = 0;

size_t process_list_insert(process_list*head,size_t depth,pid_t pgid){
    if(head == NULL){
        return 0;
    }
    jobs* new = (jobs*)malloc(sizeof(jobs));
    if(new == NULL){
        return 0;
    }
    new->depth = depth;
    new->pgid = pgid;
    current_id++;
    new->id = current_id;
    jobs* p = head->prs;
    new->next = p;
    head->prs = new;

    head->sizelist++;
    return current_id;
}

static jobs* ReturnPrevious(jobs* first, jobs* current){
    jobs* ptr = first;
    while (ptr->next != current){
        ptr = ptr->next;
    }
    return ptr;
}

static jobs* slist_find(jobs* haystack, pid_t needle){
    if (haystack == NULL ){
        return NULL;
    }
    jobs* ptr = haystack;
    while (ptr!= NULL){
        if (ptr->pgid == needle){
            return ptr;
        }
        ptr=ptr->next;
    }
    return NULL;
}

jobs * process_list_remove(process_list* head, pid_t data){
    jobs* list = head->prs;
    jobs* current = slist_find(list, data);
    if (current == NULL){
        return NULL;
    }
    else if (list == current){
        jobs* ptr = list->next;
        free(list);
        list = ptr;
        head->prs = list;
    }
    else if (current->next == NULL){
        jobs* prev = ReturnPrevious(list, current);
        prev->next = NULL;
        free(current);
    }
    else{
        jobs*prev = ReturnPrevious(list, current);
        jobs*elm = prev->next;
        prev->next = elm->next;
        head->prs = prev;
        free(current);
    }
    current_id--;
    head->sizelist--;
    return list;
}

jobs* process_list_get(process_list* head, size_t id){
    if(head == NULL){
        return NULL;
    }
    jobs* curr = head->prs;
    while( curr!= NULL){
        if(curr->id == id){
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void process_list_init(process_list* head){
    if(head == NULL){
        return;
    }
    head->prs = NULL;
    head->sizelist = 0;
}

void process_list_destroy(process_list* head){
    jobs* curr = head->prs;
    while (curr != NULL) {
        jobs* l = curr->next;
        free(curr);
        curr = l;
    }
}
void process_list_print(process_list* head){
    jobs* curr = head->prs;
    while (curr != NULL) {
        fprintf(stderr,"id=%ld pgid=%d"
                       "\n",curr->id,curr->pgid);
        curr = curr->next;
    }
}
