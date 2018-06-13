#include <assert.h>
#include <stdlib.h>

#include "list.h"

void list_init(struct list *head)
{
    head->next = head;
    head->prev = head;
}

size_t list_length(struct list *head)
{
    size_t len = 0;
    struct list *n = head->next;

    for (; n != head; n = n->next) {
        len += 1;
    }

    return len;
}

void list_append(struct list *head, struct list *n)
{
    assert(head && "invalid list pointer");
    assert(n && "invalid elt pointer");


    n->next = head;
    n->prev = head->prev;
    n->prev->next = n;
    head->prev = n;
}

void list_insert(struct list *head, struct list *n, size_t pos)
{
    assert(head && "invalid list pointer");
    assert(n && "invalid elt pointer");

    struct list *it = head->next;
    for (; pos > 0; pos--) {
        if (it == head)
            break;

        it = it->next;
    }

    n->prev = it->prev;
    n->next = it;
    n->prev->next = n;
    it->prev = n;
}

int list_remove(struct list *head, size_t pos)
{
    assert(head && "invalid list pointer");

    struct list *it = head->next;

    for (; pos > 0; pos--) {
        it = it->next;

        if (it == head)
            return 1;
    }

    it->prev->next = it->next;
    it->next->prev = it->prev;

    it->next = it;
    it->prev = it;

    return 0;
}
