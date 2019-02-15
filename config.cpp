/*
 *  config.cpp
 *  author: Aleksei Kozadaev (2019)
 */

#include "build_host.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "log.hpp"

#include "config.hpp"

//------------------------------------------------------------------------------------------------
void
usage(const char *pname)
{
    fprintf(stdout,
            "usage: %s [-h] -c <command> [-t <sec>] [-o] <file ...>\n\n"
            "   The options are as follows:\n"
            "      -h          - display this text and exit\n"
            "      -v          - display the version\n"
            "      -c <cmd>    - command to execute when event is triggered\n"
            "      -o          - exit after first iteration\n"
            "      -t <sec>    - number of seconds to skip after the last executed command "
            "(default 0)\n"
            "      -s <sec>    - number of milliseconds to sleep before reattaching in case of "
            "DELETE event (default 300)\n"
            "      <file ...>  - list of files to monitor\n\n"
            "   Please use quotes around the command if it is composed of multiple words\n\n",
                pname);
}

//------------------------------------------------------------------------------------------------
void
Config::parseArgs(int argc, char **argv)
{
    int ch;
    opterr = 0;

    char *pname = argv[0];
    if (argc == 1) {
        usage(pname);
    }

    while ((ch = getopt(argc, argv, "hvc:t:os:")) != -1) {
        switch (ch) {
            case 'h':
                usage(pname);
                exit(EXIT_SUCCESS);

            case 'v':
                printf("%s\n", VERSION);
                exit(EXIT_SUCCESS);

            case 'c':
                this->command = optarg;
                break;

            case 'o':
                this->oneRun = true;
                break;

            case 't':
                this->threshold = atoi(optarg);
                break;

            case 's':
                this->sleepDelete = atoi(optarg);
                break;

            default:
                LOG_ERR("error: unknown argument");
                exit(EXIT_FAILURE);
        }
    }

    const char **files = (const char **)argv + optind;
    this->files.insert(this->files.begin(), files, files + argc - optind);

    if (this->command.empty()) {
        LOG_ERR("error: command was not specified");
        exit(EXIT_FAILURE);
    }

    if (files == NULL) {
        LOG_ERR("error: no files to monitor");
        exit(EXIT_FAILURE);
    }
}

//------------------------------------------------------------------------------------------------
