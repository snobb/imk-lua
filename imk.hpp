#ifndef __IMK_HPP__
#define __IMK_HPP__

#include <string>
#include <vector>

struct Config {
    std::string command;
    int threshold;
    std::vector<const char *> files;

    Config() : threshold(0) {}
};

#endif /* __IMK_HPP__ */