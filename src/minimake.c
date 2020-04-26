#define _XOPEN_SOURCE
#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include "parser.h"
#include "vars_rules.h"
#include "build_targets.h"

struct minimake_opts
{
    char *filename;
    int pretty_print;
};

void print_help()
{
    puts("Usage: minimake [options] [target] ...\n"
         "Options:\n"
         "  -p: pretty-print\n"
         "  -f FILE: Read FILE as a makefile\n"
         "  -h: Print this message and exit.");
}

void parse_cmdline_opts(struct minimake_opts *opts, int argc, char **argv)
{
    int opt = 0;

    while ((opt = getopt(argc, argv, "hpf:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_help();
            exit(0);
        case 'f':
            opts->filename = optarg;
            break;
        case 'p':
            opts->pretty_print = 1;
            break;
        default:
            print_help();
            exit(2);
        }
    }
}

FILE *get_filestream_or_exit(const char *filename)
{
    FILE *f = fopen(filename, "r");

    if (!f)
        err(2, "%s", filename);

    return f;
}

FILE *get_makefile_stream(const char *filename)
{
    FILE *f = NULL;

    if (filename)
        f = get_filestream_or_exit(filename);
    else
    {
        f = fopen("makefile", "r");
        if (!f)
            f = fopen("Makefile", "r");
        if (!f)
            errx(2, "no makefile found");
    }

    return f;
}

void print_variables(struct vars_rules *vr)
{
    puts("# variables");
    for (size_t i = 0; i < vec_size(vr->variables); ++i)
    {
        struct variable *v = vec_get(vr->variables, i);
        printf("'%s' = '%s'\n", v->name, v->value);
    }
}

void print_rules(struct vars_rules *vr)
{
    puts("# rules");
    for (size_t i = 0; i < vec_size(vr->rules); ++i)
    {
        struct rule *r = vec_get(vr->rules, i);
        printf("(%s):", r->name);
        struct vec *deps = r->dependencies;
        for (size_t j = 0; j < vec_size(deps); ++j)
        {
            char *dep = vec_get(deps, j);
            printf(" [%s]", dep);
        }
        puts("");
        struct vec *cmds = r->commands;
        for (size_t j = 0; j < vec_size(cmds); ++j)
        {
            char *cmd = vec_get(cmds, j);
            printf("\t'%s'\n", cmd);
        }
    }
}

void print(struct vars_rules *vr)
{
    print_variables(vr);
    print_rules(vr);
}

int main(int argc, char **argv)
{
    struct minimake_opts opts = {0, 0};
    parse_cmdline_opts(&opts, argc, argv);
    FILE *f = get_makefile_stream(opts.filename);
    struct vars_rules *vr = parse_file(f);

    if (opts.pretty_print)
        print(vr);
    else
        build_targets(vr, optind, argc, argv);

    vr_destroy(vr);
    fclose(f);
    return 0;
}
