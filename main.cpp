/*
 *  main.cpp
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>

#include "config.hpp"
#include "poll_compat.hpp"
#include "log.hpp"

using namespace std;
using namespace imk;

//------------------------------------------------------------------------------
int
main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    Config cfg(argc, argv);
    Poll poll(cfg);

    printf(":: [%s] start monitoring: cmd[%s] files[", get_time(),
            cfg.getCommand());
    const char **files = cfg.getFiles();
    while (*files != NULL) {
        const char *file = *files++;
        printf("%s", file);
        if (*files != NULL) {
            printf(" ");
        }

        int wd = poll.regFile(file);
        cfg.addFd(wd);
    }
    printf("]\n");

    poll.dispatch();

    return 0;
}

//------------------------------------------------------------------------------
