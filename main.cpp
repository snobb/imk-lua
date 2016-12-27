/*
 *  main.cpp
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <getopt.h>

#include "imk.hpp"
#include "compat.hpp"
#include "log.hpp"

using namespace std;
using namespace imk;

void usage(const char *pname);

//------------------------------------------------------------------------------
int
main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    Config cfg;
    cfg.parseArgs(argc, argv);

    Poll poll(cfg);

    printf(":: [%s] start monitoring: cmd[%s] files[", get_time(),
            cfg.getCommand());
    const char **files = cfg.getFiles();
    while (*files != NULL) {
        printf("%s", *files++);
        if (*files != NULL) {
            printf(" ");
        }
    }
    printf("]\n");

    files = cfg.getFiles();
    while (*files != NULL) {
        int wd = poll.regFile(*files++);
        cfg.addFd(wd);
    }

    poll.dispatch();
    poll.close();

    return 0;
}

//------------------------------------------------------------------------------
void
Config::parseArgs(int argc, char **argv)
{
    int ch;
    opterr = 0;

    if (argc == 1) {
        usage(argv[0]);
    }

    while ((ch = getopt(argc, argv, "hc:")) != -1) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                exit(0);

            case 'c':
                m_cmd = optarg;
                break;

            default:
                LOG_ERR("error: unknown argument");
                exit(1);
        }
    }

    m_files = (const char**)argv + optind;

    if (m_cmd == NULL) {
        LOG_ERR("error: command was not specified");
        exit(1);
    }

    if (*m_files == NULL) {
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
