#include "process_list.h"

int process_list_insert(process_list*head,size_t depth,pid_t pgid)
{
    if(head == NULL){
        return -2;
    }
    bp* new = (bp*)malloc(sizeof(bp));
    if(new == NULL){
        return -1;
    }
    new->depth = depth;
    new->pgid = pgid;
    bp* p = head->prs;
    new->next = p;
    head->prs = new;

    head->sizelist++;
    return 0;
}

static bp* ReturnPrevious(bp* first, bp* current)
{
    bp * restrict ptr = first;
    while (ptr->next != current){
        ptr = ptr->next;
    }
    return ptr;
}

static bp* slist_find(bp* haystack, pid_t needle)///////////////
{
    if (haystack == NULL ){
        return NULL;
    }
    bp* restrict ptr = haystack;
    while (ptr!= NULL){
        if (ptr->pgid == needle){
            return ptr;
        }
        ptr=ptr->next;
    }
    return NULL;
}

bp * process_list_remove(process_list* head, pid_t data)
{
    bp* list = head->prs;
    bp* current = slist_find(list, data);
    if (current == NULL){
        return NULL;
    }
    else if (list == current){
        bp* ptr = list->next;
        free(list);
        list = ptr;
        head->prs = list;
    }
    else if (current->next == NULL){
        bp* prev = ReturnPrevious(list, current);
        prev->next = NULL;
        free(current);
    }
    else{
        bp* restrict prev = ReturnPrevious(list, current);
        bp* restrict elm = prev->next;
        prev->next = elm->next;
        head->prs = prev;
        free(current);
    }
    head->sizelist--;
    return list;
}

bp* process_list_get_pgid(process_list* head, pid_t pgid){
    if(head == NULL){
        return NULL;
    }
    bp* restrict curr = head->prs;
    while( curr!= NULL){
        if(curr->pgid == pgid){
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
    bp* restrict curr = head->prs;
    while (curr != NULL) {
        bp* l = curr->next;
        free(curr);
        curr = l;
    }
}
void process_list_print(process_list* head){
    bp* restrict curr = head->prs;
    while (curr != NULL) {
        fprintf(stderr,"pgid=%d depth= %ld"
                       "\n",curr->pgid,curr->depth);
        curr = curr->next;
    }
}
