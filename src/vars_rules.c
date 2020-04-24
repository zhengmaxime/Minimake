#include <stdlib.h>
#include <string.h>
#include "vars_rules.h"

struct variable *variable_init(char *name, char *value)
{
    struct variable *v = malloc(sizeof (*v));
    v->name = name;
    v->value = value;
    v->rec_var = NULL;
    return v;
}

struct variable *variable_init_rec(char *name, struct variable *value)
{
    struct variable *v = malloc(sizeof (*v));
    v->name = name;
    v->rec_var = value;
    v->value = NULL;
    return v;
}

void variable_destroy(struct variable *v)
{
    free(v->name);
    free(v->value);
    free(v);
}

struct rule *rule_init(char *name, struct vec *dependencies, struct vec *commands)
{
    struct rule *r = malloc(sizeof (*r));
    r->name = name;
    r->dependencies = dependencies;
    r->commands = commands;
    return r;
}

static void dep_destroy(void *dep)
{
    free(dep);
}

static void cmd_destroy(void *cmd)
{
    free(cmd);
}

void rule_destroy(struct rule *r)
{
    for (size_t i = 0; i < vec_size(r->dependencies); ++i)
        dep_destroy(vec_get(r->dependencies, i));
    for (size_t i = 0; i < vec_size(r->commands); ++i)
        cmd_destroy(vec_get(r->commands, i));
    vec_destroy(r->dependencies);
    vec_destroy(r->commands);
    free(r->name);
    free(r);
}

struct vars_rules *vr_init()
{
    struct vars_rules *vr = malloc(sizeof (*vr));
    vr->variables = vec_init(10);
    vr->rules = vec_init(10);
    vr->built_targets = vec_init(10);
    return vr;
}

void vr_add_var(struct vars_rules *vr, struct variable *v)
{
    for (size_t i = 0; i < vec_size(vr->variables); ++i)
    {
        struct variable *v2 = vec_get(vr->variables, i);
        if (!strcmp(v2->name, v->name))
        {
            variable_destroy(v2);
            vec_set(vr->variables, i, v);
            return;
        }
    }

    vec_add(vr->variables, v);
}

void vr_add_rule(struct vars_rules *vr, struct rule *r)
{
    vec_add(vr->rules, r);
}

void vr_destroy(struct vars_rules *vr)
{
    for (size_t i = 0; i < vec_size(vr->variables); ++i)
        variable_destroy(vec_get(vr->variables, i));
    for (size_t i = 0; i < vec_size(vr->rules); ++i)
        rule_destroy(vec_get(vr->rules, i));

    vec_destroy(vr->variables);
    vec_destroy(vr->rules);
    vec_destroy(vr->built_targets);
    free(vr);
}
