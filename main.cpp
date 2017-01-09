/*
 *  main.cpp
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <getopt.h>

#include "imk.hpp"
#include "poll_compat.hpp"
#include "log.hpp"

using namespace std;
using namespace imk;

void parseArgs(int argc, char **argv, Config &cfg);
void usage(const char *pname);

//------------------------------------------------------------------------------
int
main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    Config cfg;
    parseArgs(argc, argv, cfg);

    stringstream files;
    vector<const char *>::const_iterator it = cfg.files.begin();
    for (; it != prev(cfg.files.end()); ++it) {
        files << *it << " ";
    }
    files << cfg.files.back();
    printf(":: [%s] start monitoring: cmd[%s] files[%s]\n", get_time(),
            cfg.command.c_str(), files.str().c_str());

    Poll poll(cfg);
    poll.dispatch();

    return 0;
}

//------------------------------------------------------------------------------
void
parseArgs(int argc, char **argv, Config &cfg)
{
    int ch;
    opterr = 0;

    char *pname = argv[0];
    if (argc == 1) {
        usage(pname);
    }

    while ((ch = getopt(argc, argv, "hc:")) != -1) {
        switch (ch) {
            case 'h':
                usage(pname);
                exit(0);

            case 'c':
                cfg.command = optarg;
                break;

            default:
                LOG_ERR("error: unknown argument");
                exit(1);
        }
    }

    const char **files = (const char **)argv + optind;
    cfg.files = vector<const char *>(files, files + argc - optind);

    if (cfg.command.empty()) {
        LOG_ERR("error: command was not specified");
        exit(1);
    }

    if (files == NULL) {
        LOG_ERR("error: no files to monitor");
        exit(1);
    }
}

//------------------------------------------------------------------------------
void
usage(const char *pname)
{
    fprintf(stdout,
            "usage: %s [-h] -c <command> <file ...>\n\n"
            "   The options are as follows:\n"
            "      -h          - display this text and exit\n"
            "      -c <cmd>    - command to execute when event is triggered\n"
            "      <file ...>  - list of files to monitor\n\n"
            "   Please use quotes around the command if it is composed of "
            "multiple words\n\n", pname);
}

//------------------------------------------------------------------------------
