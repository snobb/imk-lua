/*
 *  main.cpp
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <getopt.h>

#include "context.hpp"
#include "log.hpp"

using namespace std;
using namespace imk;

static void parseArgs(int argc, char **argv);
static void usage(const char *pname);

Context ctx;

//------------------------------------------------------------------------------
int
main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    parseArgs(argc, argv);

    stringstream files;
    if (ctx.files().empty()) {
        LOG_ERR("no files to monitor");
        exit(1);
    }

    auto it = ctx.files().cbegin();
    files << it->second;
    for (++it; it != ctx.files().cend(); ++it) {
        files << " " << it->second;
    }
    printf(":: [%s] start monitoring: cmd[%s] files[%s]\n", get_time(),
            ctx.command().empty() ? "none" : ctx.command().c_str(),
            files.str().c_str());

    ctx.dispatch();

    return 0;
}

//------------------------------------------------------------------------------
static void
parseArgs(int argc, char **argv)
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
                ctx.command(optarg);
                break;

            default:
                LOG_ERR("error: unknown argument");
                exit(1);
        }
    }

    const char **files = (const char **)argv + optind;
    if (*files == NULL) {
        LOG_ERR("error: no files to monitor");
        exit(1);
    }

    while(*files != NULL) {
        ctx.registerFile(*files++);
    }
}

//------------------------------------------------------------------------------
static void
usage(const char *pname)
{
    fprintf(stdout,
            "usage: %s [-h] -c <command> <file ...>\n\n"
            "   The options are as follows:\n"
            "      -h          - display this text and exit\n"
            "      -c <cmd>    - command to execute when event is triggered\n"
            "      <file ...>  - list of files to monitor\n\n"
            "   Please use quotes around the command if it is composed of "
            "multiple words\n\n"
            "   Lua event handlers are searched in .luarc.lua file in the "
            "current directory and then in $HOME\n\n", pname);
}

//------------------------------------------------------------------------------
