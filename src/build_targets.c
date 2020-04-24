#include <err.h>
#include <string.h>
#include <stdlib.h>
#include "build_targets.h"
#include "exec_command.h"
#include "log_command.h"
#include "vector.h"
#include "vars_rules.h"
#include "var_sub.h"

static int is_target_built(struct vec *built_targets, char *arg)
{
    for (size_t i = 0; i < vec_size(built_targets); ++i)
    {
        char *target_name = vec_get(built_targets, i);
        if (!strcmp(target_name, arg))
            return 1;
    }
    return 0;
}

static struct rule *find_rule(struct vars_rules *vr, char *arg)
{
    for (size_t i = 0; i < vec_size(vr->rules); ++i)
    {
        struct rule *r = vec_get(vr->rules, i);
        if (!strcmp(r->name, arg))
            return r;
    }
    errx(2, " *** No rule to make target '%s'. Stop.", arg);
}

void build_target(struct vars_rules *vr, struct rule *r)
{
    struct vec *commands = r->commands;
    for (size_t i = 0; i < vec_size(commands); ++i)
    {
        char *cmd = vec_get(commands, i);
        int new = 0;
        cmd = substitute_vars(vr, cmd, &new);
        log_command(cmd);
        exec_command(cmd);
        if (new)
            free(cmd);
    }
    return;
}

static void build_first_standard_target(struct vars_rules *vr)
{
    for (size_t i = 0; i < vec_size(vr->rules); ++i)
    {
        struct rule *r = vec_get(vr->rules, i);
        if (!strchr(r->name, '%'))
        {
            build_target(vr, r);
            return;
        }
    }
    errx(2, " *** No targets. Stop.");
}

static int is_target_built(struct vec *built_targets, char *arg)
{
    for (size_t i = 0; i < vec_size(built_targets); ++i)
    {
        char *target_name = vec_get(built_targets, i);
        if (!strcmp(target_name, arg))
            return 1;
    }
    return 0;
}

void build_targets(struct vars_rules *vr, int optind, int argc, char **argv)
{
    if (optind == argc) // no rule given
    {
        build_first_standard_target(vr);
    }
    else
    {
        struct vec *built_targets = vec_init(10);
        for (int i = optind; i < argc; ++i)
        {
            if (is_target_built(built_targets, argv[i]))
            {
                warnx("'%s' is up to date.", argv[i]);
                continue;
            }
            struct rule *r = find_rule(vr, argv[i]);
            build_target(vr, r);
            vec_add(built_targets, argv[i]);
        }
        vec_destroy(built_targets);
    }
}
