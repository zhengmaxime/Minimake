#include <sys/stat.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "build_targets.h"
#include "find_rule.h"
#include "exec_command.h"
#include "log_command.h"
#include "vector.h"
#include "vars_rules.h"
#include "var_sub.h"

#define UP_TO_DATE 1
#define NOTIF_UP_TO_DATE 1
#define CALLED_FROM_CMDLINE 1

static int build_target(struct vars_rules *vr,
                        char *target,
                        int warn_uptodate,
                        int called_from_cmdline);

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

static int dep_more_recent_than_file(struct stat *dep_statbuf,
                                     struct stat *target_statbuf)
{
    time_t dep = dep_statbuf->st_mtime;
    time_t target = target_statbuf->st_mtime;
    return dep > target;
}

/* return true if one of the deps is more recent than the current file */
static int build_deps(struct vars_rules *vr,
                      struct rule *r,
                      char *target,
                      int *nb_executed_cmd)
{
    struct stat target_statbuf;
    if (target)
        stat(target, &target_statbuf);

    int more_recent = 0;

    for (size_t i = 0; i < vec_size(r->dependencies); ++i)
    {
        char *dep = vec_get(r->dependencies, i);
        struct stat dep_statbuf;

        if ((stat(dep, &dep_statbuf) != 0) // dep file doesn't exist
            || !target // target file doesn't exist
            || (dep_more_recent_than_file(&dep_statbuf, &target_statbuf)))
        {
            *nb_executed_cmd += build_target(vr, dep, !NOTIF_UP_TO_DATE,
                                             !CALLED_FROM_CMDLINE);
            more_recent = 1;
            continue;
        }
    }

    return more_recent;
}

/* return true if rule was up to date */
static int build_rule(struct vars_rules *vr,
                      struct rule *r,
                      char *target,
                      int *nb_executed_cmd)
{
    struct stat statbuf;
    int file_exists = (stat(target, &statbuf) == 0);

    if (!r->phony && file_exists)
    {
        if (vec_size(r->dependencies) == 0)
            return UP_TO_DATE;
        else
        {
            int is_more_recent = build_deps(vr, r, target, nb_executed_cmd);
            if (!is_more_recent)
                return UP_TO_DATE;
        }
    }

    if ((r->phony || !file_exists) && (vec_size(r->dependencies) > 0))
        build_deps(vr, r, NULL, nb_executed_cmd);

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
    *nb_executed_cmd += vec_size(r->commands);

    return !UP_TO_DATE;
}

/* return nb of commands executed in deps and recipe */
static int build_target(struct vars_rules *vr,
                        char *target,
                        int warn_uptodate,
                        int called_from_cmdline)
{
    int nb_executed_cmd = 0;

    struct rule *r = find_rule(vr, target);

    if (is_target_built(vr->built_targets, target))
    {
        if (r->phony)
            printf("minimake: Nothing to be done for '%s'.\n", target);
        else if (warn_uptodate)
            printf("minimake: '%s' is up to date.\n", target);
        return nb_executed_cmd;
    }

    int was_uptodate = build_rule(vr, r, target, &nb_executed_cmd);

    if (called_from_cmdline
            && nb_executed_cmd == 0 && vec_size(r->commands) == 0)
    {
        printf("minimake: Nothing to be done for '%s'.\n", target);
    }
    else if (was_uptodate)
    {
        if (warn_uptodate)
            printf("minimake: '%s' is up to date.\n", target);
    }

    vec_add(vr->built_targets, target);

    return nb_executed_cmd;
}

static void build_first_standard_target(struct vars_rules *vr)
{
    int nb_executed_cmd = 0;
    for (size_t i = 0; i < vec_size(vr->rules); ++i)
    {
        struct rule *r = vec_get(vr->rules, i);
        char *target = r->name;
        if (!strchr(target, '%'))
        {
            int was_uptodate = build_rule(vr, r, target, &nb_executed_cmd);
            if (nb_executed_cmd == 0 && vec_size(r->commands) == 0)
                printf("minimake: Nothing to be done for '%s'.\n", target);
            else if (was_uptodate)
                printf("minimake: '%s' is up to date.\n", target);
            return;
        }
    }
    errx(2, " *** No targets. Stop.");
}

void build_targets(struct vars_rules *vr, int optind, int argc, char **argv)
{
    if (optind == argc) // no rule given
        build_first_standard_target(vr);
    else
    {
        for (int i = optind; i < argc; ++i)
            build_target(vr, argv[i], NOTIF_UP_TO_DATE, CALLED_FROM_CMDLINE);
    }
}
