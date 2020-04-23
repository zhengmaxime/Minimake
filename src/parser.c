#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <err.h>
#include "vars_rules.h"

#define UNK 0
#define VARDEF 1
#define RULE 2

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

static char *get_value(char *line)
{
    char *ptr = skip_spaces(line);
    size_t value_len = strlen(ptr + 1);
    char *value = calloc(1, value_len + 1);
    strncpy(value, ptr, value_len);
    return value;
}

static size_t get_name_len(char *ptr)
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

static char *get_name(char *line)
{
    line = skip_spaces(line);
    size_t name_len = get_name_len(line);
    char *name = calloc(1, name_len + 1);
    strncpy(name, line, name_len);
    return name;
}

static int parse_vardef(struct vars_rules *vr, char *line)
{
    skip_comment(line);
    char *sep = strchr(line, '=');
    if (!sep)
        return 0;

    char *value = get_value(sep + 1);
    char *name = get_name(line);

    struct variable *var = variable_init(name, value);
    vr_add_var(vr, var);
    return 1;
}

static struct vec *parse_dependencies(char *line)
{
    struct vec *deps = vec_init(10);

    line = skip_spaces(line);

    char *dep = strtok(line, " \t\n");
    while (dep)
    {
        vec_add(deps, strdup(dep));
        dep = strtok(NULL, " \t\n");
    }

    return deps;
}

static int parse_rule(struct vars_rules *vr, char *line)
{
    skip_comment(line);
    char *sep = strchr(line, ':');
    if (!sep)
        return 0;

    char *name = get_name(line);

    struct vec *dependencies = parse_dependencies(sep + 1);
    struct vec *commands = vec_init(10);
    struct rule *r = rule_init(name, dependencies, commands);
    vr_add_rule(vr, r);
    return RULE;
}

static int check_blank_line(char *line)
{
    for (; line; ++line)
    {
        if (!isspace(*line))
            return 0;
    }
    return 1;
}

static int parse_command(struct vars_rules *vr, char *line)
{
    if (check_blank_line(line))
        return RULE;

    if (line[0] != '\t')
        return UNK;

    line = skip_spaces(line);
    char *cmd = get_value(line);
    struct rule *r = vec_get(vr->rules, vec_size(vr->rules) - 1);
    vec_add(r->commands, cmd);

    return RULE;
}

struct vars_rules *parse_file(FILE *f)
{
    struct vars_rules *vr = vr_init();

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    int type = UNK;

    while ((nread = getline(&line, &len, f)) != -1)
    {
        if (type == RULE) // EXPECT CMD
        {
            type = parse_command(vr, line);
            if (type == RULE)
                continue;
        }
        type = parse_vardef(vr, line);
        if (type == VARDEF)
            continue;
        type = parse_rule(vr, line);
    }
    free(line);

    return vr;
}
