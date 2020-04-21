#define _XOPEN_SOURCE
#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void print_help()
{
    puts("Usage: minimake [options] [target] ...\n"
         "Options:\n"
         "  -p: pretty-print\n"
         "  -f FILE: Read FILE as a makefile\n"
         "  -h: Print this message and exit.");
}

void parse_cmdline_opts(int argc, char **argv)
{
    int opt = 0;

    while ((opt = getopt(argc, argv, "hpf:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_help();
            exit(0);
        default:
            print_help();
            exit(2);
        }
    }
}

int main(int argc, char **argv, char **envp)
{
    parse_cmdline_opts(argc, argv);
    (void)envp;
    return 0;
}
