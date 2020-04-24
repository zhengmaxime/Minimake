#include <stdio.h>
#include <ctype.h>
#include <err.h>

static inline char *skip_spaces(char *line)
{
    while (isspace(*line))
        ++line;
    return line;
}

void log_command(char *cmd)
{
    cmd = skip_spaces(cmd);
    if (cmd[0] == 0 || cmd[0] == '@')
        return;
    puts(cmd);
    fflush(NULL);
}
