#ifndef K_LINKED_LIST_H
#define K_LINKED_LIST_H

#include <kern/lib.h>

#define ll_init(head_ptr) do { \
    (head_ptr)->next = (void*)(head_ptr); \
    (head_ptr)->prev = (void*)(head_ptr); \
} while(0)

#define ll_insert(head_ptr, node) do { \
    assert((node)->next == NULL); \
    assert((node)->prev == NULL); \
    (node)->next = (head_ptr)->next; \
    (node)->prev = (void*)head_ptr; \
    (head_ptr)->next = (node); \
    (node)->next->prev = (node); \
} while(0)

#define ll_unlink(node) do { \
    if((node)->next) { \
        (node)->next->prev = (node)->prev; \
        (node)->next = NULL; \
    } \
    if((node)->prev) { \
        (node)->prev->next = (node)->next; \
        (node)->prev = NULL; \
    } \
} while(0)

#endif