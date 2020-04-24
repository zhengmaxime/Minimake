#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include "exec_command.h"

static inline char *skip_spaces_and_log(char *line)
{
    while (line && isspace(*line))
        ++line;
    if (line[0] == '@')
        ++line;
    return line;
}

static char **build_args(char *command)
{
    char **args = calloc(4, sizeof (char *));
    args[0] = "/bin/sh";
    args[1] = "-c";
    args[2] = skip_spaces_and_log(command);
    args[3] = NULL;
    return args;
}

void exec_command(char *command)
{
    char **args = build_args(command);
    pid_t pid = fork();
    if (pid == -1)
        err(2, "fork failed");
    if (pid == 0)
    {
        if (-1 == execve("/bin/sh", args, environ))
            err(2, "execve failed");
    }
    else
    {
        free(args);
        int status = 0;
        if (-1 == waitpid(pid, &status, 0))
            err(2, "waitpid failed");
        if (status != 0)
            errx(2, " *** Error %d", WEXITSTATUS(status));
    }
}
