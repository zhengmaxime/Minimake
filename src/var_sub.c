#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "vars_rules.h"

static inline int is_single_character(char *var)
{
    return ((*var == 0) || (*var == '\n') || (*var != '(' && *var != '{'));
}

static inline char get_closing_bracket(char open)
{
    return (open == '(') ? ')' : '}';
}

static char *get_var_name(char *dollar, char **end)
{
    char *var_name = NULL;

    char *ptr = dollar + 1;
    if (is_single_character(ptr))
    {
        *end = ptr + 1;
        if (*ptr == 0 || *ptr == '\n') // $\n=$
            --ptr;
        var_name = strndup(ptr, 1);
        return var_name;
    }

    if (isspace(dollar[1]) || dollar[1] == ':')
        errx(2, "$ with bad character after");

    char closing = get_closing_bracket(*ptr);
    *end = strchr(ptr, closing);
    if (*end == NULL)
        errx(2, "var not closed");
    *end = *end + 1;
    var_name = strndup(ptr + 1, (*end - 2) - ptr);
    return var_name;
}

static char *get_var_value(struct vars_rules *vr, char *var_name)
{
    if (!strcmp(var_name, "$"))
        return strdup(var_name);

    for (size_t i = 0; i < vec_size(vr->variables); ++i)
    {
        struct variable *v = vec_get(vr->variables, i);
        if (!strcmp(v->name, var_name))
            return strdup(v->value);
        // FIMXE: rec var
    }

    char *empty_string = calloc(1, 1);
    return empty_string;
}

static char *substitute_var(struct vars_rules *vr, char *line, char *var_dollar)
{
    char *var_after = NULL;
    char *var_name = get_var_name(var_dollar, &var_after);
    char *var_value = get_var_value(vr, var_name);

    // allocate a new line, big enough
    char *dest = calloc(1, strlen(line) + strlen(var_value) + 1);
    strncat(dest, line, var_dollar - line);
    strncat(dest, var_value, strlen(var_value));
    strncat(dest, var_after, strlen(var_after));

    free(var_name);
    free(var_value);

    return dest;
}

static char *expand_double_dollar(char *line)
{
    if (line == NULL || *line == 0)
        return line;

    for (char *ptr = line + 1; ptr && *ptr != '\0'; ++ptr)
    {
        if (ptr[-1] == '$' && ptr[0] == '$')
        {
            memmove(ptr - 1, ptr, 1 + strlen(ptr));
        }
    }

    return line;
}

char *substitute_vars(struct vars_rules *vr, char *line, int *new)
{
    char *old_line = strdup(line); // line is freed in parse_file
    char *new_line = NULL;
    char *partial_line = NULL;

    old_line = expand_double_dollar(old_line);
    char *var_dollar = strrchr(old_line, '$');

    while (var_dollar)
    {
        new_line = substitute_var(vr, old_line, var_dollar);
        free(old_line);

        partial_line = strndup(new_line, var_dollar - old_line);
        var_dollar = strrchr(partial_line, '$'); // avoid infinite loop
        free(partial_line);

        if (var_dollar == NULL)
            break;

        var_dollar = new_line + (var_dollar - partial_line);
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
