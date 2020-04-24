#include <err.h>
#include <string.h>
#include <stdlib.h>
#include "build_targets.h"
#include "exec_command.h"
#include "log_command.h"
#include "vector.h"
#include "vars_rules.h"
#include "var_sub.h"

static void build_target(struct vars_rules *vr, char *target,
        int warn_uptodate);

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

static void build_deps(struct vars_rules *vr, struct vec *deps)
{
    int warn_uptodate = 0;
    for (size_t i = 0; i < vec_size(deps); ++i)
    {
        char *dep = vec_get(deps, i);
        build_target(vr, dep, warn_uptodate);
    }
}

static void build_rule(struct vars_rules *vr, struct rule *r)
{
    build_deps(vr, r->dependencies);
    for (size_t i = 0; i < vec_size(r->commands); ++i)
    {
        char *cmd = vec_get(r->commands, i);
        int new = 0;
        cmd = substitute_vars(vr, cmd, &new);
        log_command(cmd);
        exec_command(cmd);
        if (new)
            free(cmd);
    }
}

static void build_target(struct vars_rules *vr, char *target, int warn_uptodate)
{
    if (is_target_built(vr->built_targets, target))
    {
        if (warn_uptodate)
            warnx("'%s' is up to date.", target);
        return;
    }
    struct rule *r = find_rule(vr, target);
    build_rule(vr, r);
    vec_add(vr->built_targets, target);
}

static void build_first_standard_target(struct vars_rules *vr)
{
    for (size_t i = 0; i < vec_size(vr->rules); ++i)
    {
        struct rule *r = vec_get(vr->rules, i);
        if (!strchr(r->name, '%'))
        {
            build_rule(vr, r);
            return;
        }
    }
    errx(2, " *** No targets. Stop.");
}

void build_targets(struct vars_rules *vr, int optind, int argc, char **argv)
{
    if (optind == argc) // no rule given
    {
        build_first_standard_target(vr);
    }
    else
    {
        int warn_uptodate = 1;
        for (int i = optind; i < argc; ++i)
        {
            build_target(vr, argv[i], warn_uptodate);
        }
    }
}
