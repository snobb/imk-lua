#ifndef __IMK_HPP__
#define __IMK_HPP__

#include <string>
#include <vector>

struct Config {
    std::string command;
    std::vector<const char *> files;
};

#endif /* __IMK_HPP__ */
