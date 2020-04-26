#include <stdlib.h>
#include <stdio.h>
#include "stack.h"

struct stack
{
    char elt;
    struct stack *next;
};

struct stack *stack_push(struct stack *st, char e)
{
    struct stack *new_top = malloc(sizeof(struct stack));
    new_top->elt = e;
    new_top->next = st;
    return new_top;
}

char stack_top(struct stack *st)
{
    if (st == NULL)
        return 0;
    return st->elt;
}

struct stack *stack_pop(struct stack *st)
{
    if (st == NULL)
        return NULL;
    struct stack *tmp = st->next;
    free(st);
    return tmp;
}

void stack_destroy(struct stack *st)
{
    struct stack *tmp = NULL;
    while (st)
    {
        tmp = st->next;
        free(st);
        st = tmp;
    };
}
