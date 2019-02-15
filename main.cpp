/*
 *  main.cpp
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <sstream>

#include "config.hpp"
#include "poll_compat.hpp"
#include "log.hpp"

using namespace std;
using namespace imk;

void parseArgs(int argc, char **argv, Config &cfg);
void usage(const char *pname);

//------------------------------------------------------------------------------------------------
int
main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    Config cfg;
    cfg.parseArgs(argc, argv);

    stringstream files;
    auto it = cfg.files.cbegin();
    for (; it != prev(cfg.files.cend()); ++it) {
        files << *it << " ";
    }
    files << cfg.files.back();
    printf(":: [%s] start monitoring: cmd[%s] threshold[%d] files[%s]\n", get_time(),
            cfg.command.c_str(), cfg.threshold, files.str().c_str());

    Poll poll(cfg);
    poll.dispatch();

    return 0;
}

//------------------------------------------------------------------------------------------------
