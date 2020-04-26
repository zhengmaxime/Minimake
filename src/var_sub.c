#define _GNU_SOURCE
#include <unistd.h>
#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "vars_rules.h"
#include "var_sub.h"
#include "stack.h"

static inline int is_single_character(char *var)
{
    return ((*var == '$') || (*var == '\n') || (*var != '(' && *var != '{'));
}

static inline char *get_closing_bracket(char *ptr)
{
    struct stack *st = NULL;

    for (; *ptr; ++ptr)
    {
        char c = ptr[0];

        if (ptr[-1] == '$' && (c == '(' || c == '{'))
            st = stack_push(st, c);

        else if (c == ')' || c == '}')
        {
            char top = stack_top(st);
            if ((top == '(' && c == ')') || (top == '{' && c == '}'))
                st = stack_pop(st);

            if (st == NULL)
                return ptr;
        }
    }

    errx(2, "var not closed properly");
}

static char *get_var_name(struct vars_rules *vr, char *dollar, char **end)
{
    char *var_name = NULL;

    char *ptr = dollar + 1;
    if (is_single_character(ptr))
    {
        *end = ptr + 1;
        if (*ptr == '\n') // $\n=$
            --ptr;
        var_name = strndup(ptr, 1);
        return var_name;
    }

    if (isspace(dollar[1]) || dollar[1] == ':')
        errx(2, "$ with bad character after");

    *end = get_closing_bracket(ptr);
    if (*end == NULL)
        errx(2, "var not closed");
    *end = *end + 1;
    var_name = strndup(ptr + 1, (*end - 2) - ptr);
    int new = 0;
    char *new_var_name = substitute_vars(vr, var_name, &new);
    if (new)
    {
        free(var_name);
        var_name = new_var_name;
    }
    return var_name;
}

static char *get_var_value(struct vars_rules *vr, char *var_name)
{
    if (!strcmp(var_name, "$")) // handle $$
        return strdup("$");

    for (size_t i = 0; i < vec_size(vr->variables); ++i)
    {
        struct variable *v = vec_get(vr->variables, i);
        if (!strcmp(v->name, var_name))
            return strdup(v->value);
    }

    extern char **environ;
    for (size_t i = 0; environ[i]; ++i)
    {
        size_t len = strlen(var_name);
        if (!strncmp(var_name, environ[i], len))
            return strdup(environ[i] + len + 1);
    }

    char *empty_string = calloc(1, 1);
    return empty_string;
}

static char *substitute_var(struct vars_rules *vr,
                            char *line,
                            char *var_dollar,
                            char **next)
{
    char *var_after = NULL;
    char *var_name = get_var_name(vr, var_dollar, &var_after);
    char *var_value = get_var_value(vr, var_name);

    if (strcmp(var_value, "$")) // $ is $, no recursion there...
    {
        int new = 0;
        char *new_var_value = substitute_vars(vr, var_value, &new);
        if (new)
        {
            free(var_value);
            var_value = new_var_value;
        }
    }

    // allocate a new line, big enough
    char *dest = calloc(1, strlen(line) + strlen(var_value) + 1);
    strncat(dest, line, var_dollar - line);
    strncat(dest, var_value, strlen(var_value));
    strncat(dest, var_after, strlen(var_after));
    *next = dest + (var_dollar - line) + strlen(var_value) + 1;

    free(var_name);
    free(var_value);

    return dest;
}

char *substitute_vars(struct vars_rules *vr, char *line, int *new)
{
    char *old_line = strdup(line); // line is freed in parse_file
    char *new_line = NULL;

    char *var_dollar = strchr(old_line, '$');
    while (var_dollar)
    {
        char *next = NULL;
        new_line = substitute_var(vr, old_line, var_dollar, &next);
        free(old_line);
        var_dollar = strchr(next, '$');
        old_line = new_line;
    }
    if (new_line)
    {
        *new = 1;
        line = new_line;
    }
    else
        free(old_line); // dup for nothing...

    return line;
}
