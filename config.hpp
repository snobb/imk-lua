#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <string>
#include <vector>

struct Config {
    std::string command;
    int threshold;
    int oneRun;
    unsigned sleepDelete;
    std::vector<std::string> files;

    Config() : threshold(0), oneRun(false), sleepDelete(300) {}
    void parseArgs(int argc, char **argv);
};

#endif /* __CONFIG_HPP__ */
