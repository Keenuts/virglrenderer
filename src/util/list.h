#pragma once

#include "stddef.h"

struct list {
    struct list *next;
    struct list *prev;
};


#define LIST_FOR_EACH(Var, List, Field) \
    for (Var = CONTAINER_OF(List.next, typeof(*Var), Field) ; \
         &(Var->Field) != &List; \
         Var = CONTAINER_OF(Var->Field.next, typeof(*Var), Field))

#define CONTAINER_OF(List, Type, Field) \
    (Type*)((char*)List - offsetof(Type, Field))


#define LIST_FRONT(Var, List, Field) \
    it = CONTAINER_OF(List.next, typeof(*Var), Field)

#define LIST_BACK(Var, List, Field) \
    it = CONTAINER_OF(List.prev, typeof(*Var), Field)

void list_init(struct list *head);

size_t  list_length(struct list *head);
void    list_append(struct list *head, struct list *n);
void    list_insert(struct list *head, struct list *n, size_t pos);
int     list_remove(struct list *head, size_t pos);
