#pragma once

struct stack;

struct stack *stack_push(struct stack *st, char e);
char stack_top(struct stack *st);
struct stack *stack_pop(struct stack *st);
void stack_destroy(struct stack *st);
