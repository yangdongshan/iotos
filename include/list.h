#ifndef __LIST_H
#define __LIST_H

#include <types.h>

#define offset_of(type, member) \
    ((unsigned long) &((s *) 0)->m)

#define container_of(ptr, type, member) \
    ((type*)((unsigned long)(ptr) - offsetof(type, member)))


struct list_node {
    struct list_node *prev;
    struct list_node *next;
};

#define LIST_INITIAL_VALUE(list) {&(list), &(list)}

#define LIST_INITIAL_CLEARED_VALUE() {NULL, NULL}

static inline void list_head_init(struct list_node *head)
{
    head->prev = head->next = head;
}

static inline void list_clear_node(struct list_node *list)
{
    list->prev = list->next = NULL;
}

static inline bool list_in_list(struct list_node *node)
{
    if (node->prev == NULL && node->next == NULL)
        return false;
    else
        return true;
}

static inline void list_add_head(struct list_node *head, struct list_node *node)
{
    node->prev = head->prev;
    node->next = head;
    head->prev->next = node;
    head->prev = node;
}

#define list_add_after(head, node) list_add_head(head, node)

static inline void list_add_tail(struct list_node *head, struct list_node *node)
{
    node->prev = head->prev;
    node->next = head;
    head->prev->next = node;
    head->prev = node;
}

#define list_add_before(head, node) list_add_tail(head, node)

static inline void list_delete(struct list_node *node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->prev = node->next = NULL;
}

static inline struct list_node* list_remove_head(struct list_node *head)
{
    if (head->next != head) {
        struct list_node *node = head->next;
        list_delete(node);
        return node;
    } else {
        return NULL;
    }
}

static inline struct list_node* list_remove_tail(struct list_node *head)
{
    if (head->prev != head) {
        struct list_node *node = head->prev;
        list_delete(node);
        return node;
    } else {
        return NULL;
    }
}


static inline struct list_node* list_peek_head(struct list_node *head)
{
    if (head->next != head) {
        return head->next;
    } else {
        return NULL;
    }
}


static inline struct list_node* list_peek_tail(struct list_node *head)
{
    if (head->prev != head) {
        return head->prev;
    } else {
        return NULL;
    }
}

static inline struct list_node* list_prev(struct list_node *head)
{
    if (head->prev != head) {
        return head->prev;
    } else {
        return NULL;
    }
}


static inline struct list_node* list_next(struct list_node *head)
{
    if (head->next != head) {
        return head->next;
    } else {
        return NULL;
    }
}

static inline bool list_is_empty(struct list_node *head)
{
    return (head->next == head)? true: false;
}


#define list_foreach(head, node) \
    for (node = (head)->next; node != (head); node = (node)->next)

#define list_foreach_safe(head, node, temp) \
    for (node = (head)->next, temp = (node)->next; \
        node != (head); \
        node = temp, temp = (node)->next)

#define list_foreach_entry(head, entry, type, member) \
    for ((entry) = container_of((head)->next, type, member); \
         &(entry)->member != head; \
        (entry) = container_of((entry)->member.next, type, member))

#define list_foreach_entry_safe(head, entry, temp, type, member) \
    for ((entry) = container_of((head)->next, type, member), \
         (temp) = container_of((entry)->member.next, type, member); \
         &(entry)->member != head; \
        (entry) = (temp), (temp) = container_of((entry)->member.next, type, member))

static inline size_t list_length(struct list_node *head)
{
    size_t cnt = 0;
    struct list_node *node;
    list_foreach(head, node) {
        cnt++;
    }

    return cnt;
}


#endif // LIST_H

