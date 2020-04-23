#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <err.h>
#include "vars_rules.h"

static inline void skip_comment(char *line)
{
    char *comment = strchr(line, '#');
    if (comment)
    {
        *comment = '\n';
        *(comment + 1) = '\0';
    }
}

static inline char *skip_spaces(char *line)
{
    while (isspace(*line))
        ++line;
    return line;
}

static char *get_var_value(char *sep)
{
    char *ptr = skip_spaces(sep + 1);
    size_t value_len = strlen(ptr + 1);
    char *value = calloc(1, value_len + 1);
    strncpy(value, ptr, value_len);
    return value;
}

static inline size_t get_var_name_len(char *ptr)
{
    size_t name_len = 0;
    for (; ptr && *ptr != '=' && !isspace(*ptr) && *ptr != ':'; ++ptr)
    {
        if (*ptr == ':')
            errx(2, "vardec parsing error");
        ++name_len;
    }
    return name_len;
}

static char *get_var_name(char *line)
{
    line = skip_spaces(line);
    size_t name_len = get_var_name_len(line);
    char *name = calloc(1, name_len + 1);
    strncpy(name, line, name_len);
    return name;
}

static void parse_vardef(struct vars_rules *vr, char *line)
{
    skip_comment(line);
    char *sep = strchr(line, '=');
    if (!sep)
        return;

    char *value = get_var_value(sep);
    char *name = get_var_name(line);

    struct variable *var = variable_init(name, value);
    vr_add_var(vr, var);
}

struct vars_rules *parse_file(FILE *f)
{
    struct vars_rules *res = vr_init();

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&line, &len, f)) != -1)
    {
        parse_vardef(res, line);
    }
    free(line);

    return res;
}
