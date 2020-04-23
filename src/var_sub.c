#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "vars_rules.h"

static inline int is_single_character(char *var)
{
    return ((*var == '$') || (*var != '(' && *var != '{'));
}

static inline char get_closing_bracket(char open)
{
    return (open == '(') ? ')' : '}';
}

static char *get_var_name(char *dollar, char **end)
{
    if (!dollar[1] || isspace(dollar[1])
        || dollar[1] == ':' || dollar[1] == ':')
        errx(2, "$ with bad character after");

    char *var_name = NULL;

    char *ptr = dollar + 1;
    if (is_single_character(ptr))
    {
        *end = ptr + 1;
        var_name = strndup(ptr, 1);
        return var_name;
    }

    char closing = get_closing_bracket(*ptr);
    *end = strchr(ptr + 1, closing);
    if (*end == NULL)
        errx(2, "var not closed");
    var_name = strndup(ptr, *end - ptr - 1);
    return var_name;
}

static char *get_var_value(struct vars_rules *vr, char *var_name)
{
    if (!strcmp(var_name, "$"))
        return var_name;

    for (size_t i = 0; i < vec_size(vr->variables); ++i)
    {
        struct variable *v = vec_get(vr->variables, i);
        if (!strcmp(v->name, var_name))
            return v->value;
        // FIMXE: rec var
    }

    char *empty_string = calloc(1, 1);
    return empty_string;
}

char *substitute_var(struct vars_rules *vr, char *line, char *var_dollar)
{
    char *var_end = NULL;
    char *var_name = get_var_name(var_dollar, &var_end);
    char *var_value = get_var_value(vr, var_name);

    // allocate a new line, big enough
    char *dest = calloc(1, strlen(line) + strlen(var_value) + 1);
    strncat(dest, line, var_dollar - line);
    strncat(dest, var_value, strlen(var_value));
    strcat(dest, var_end);
    free(line);
    return dest;
}
