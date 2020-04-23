#pragma once
#include "vector.h"

struct variable
{
    char *name;
    char *value;
    struct variable *rec_var;
};

struct rule
{
    char *name;
    struct vec *dependencies;
    struct vec *commands;
};

struct vars_rules
{
    struct vec *variables;
    struct vec *rules;
};

struct variable *variable_init(char *name, char *value);

struct variable *variable_init_rec(char *name, struct variable *value);

void variable_destroy(struct variable *v);

struct rule *rule_init(char *name, struct vec *dependencies, struct vec *commands);

void rule_destroy(struct rule *r);

struct vars_rules *vr_init();

void vr_add_var(struct vars_rules *vr, struct variable *v);

void vr_add_rule(struct vars_rules *vr, struct rule *r);

void vr_destroy(struct vars_rules *vr);
