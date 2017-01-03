/*
 *  config.cpp
 *  author: Aleksei Kozadaev (2017)
 */

#include <stdlib.h>
#include <getopt.h>

#include "log.hpp"
#include "config.hpp"

using namespace imk;

void
Config::addFd(int fd)
{
    m_fds.push_back(fd);
}

void
Config::setFd(size_t idx, int fd)
{
    if (idx >= m_fds.size()) { return; }
    m_fds[idx] = fd;
}

ssize_t
Config::getFdIndex(int fd)
{
    std::vector<int>::const_iterator it =
        find(m_fds.begin(), m_fds.end(), fd);
    return (it == m_fds.end() ? -1 : (it - m_fds.begin()));
}

void
Config::parseArgs(int argc, char **argv)
{
    int ch;
    opterr = 0;

    m_pname = argv[0];
    if (argc == 1) {
        usage();
    }

    while ((ch = getopt(argc, argv, "hc:")) != -1) {
        switch (ch) {
            case 'h':
                usage();
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

void
Config::usage()
{
    fprintf(stdout,
            "usage: %s [-h] -c <command> <file ...>\n\n"
            "   The options are as follows:\n"
            "      -h          - display this text and exit\n"
            "      -c <cmd>    - command to execute when event is triggered\n"
            "      <file ...>  - list of files to monitor\n\n"
            "   Please use quotes around the command if it is composed of "
            "multiple words\n\n", m_pname);
}

