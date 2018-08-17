#ifndef __IMK_HPP__
#define __IMK_HPP__

#include <string>
#include <vector>

struct Config {
    std::string command;
    int threshold;
    int oneRun;
    unsigned sleepDelete;
    std::vector<std::string> files;

    Config() : threshold(0), oneRun(false), sleepDelete(300) {}
};

#endif /* __IMK_HPP__ */
