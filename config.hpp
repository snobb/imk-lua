#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <vector>
#include <algorithm>

namespace imk {

class Config
{
public:
    Config(int argc, char **argv) : m_cmd(NULL), m_files(NULL)
    {
        parseArgs(argc, argv);
    }

    ~Config() {}

    const char *getCommand() const { return m_cmd; }
    const char **getFiles() const { return m_files; }

    void addFd(int fd);
    void setFd(size_t idx, int fd);

    ssize_t getFdIndex(int fd);
    const std::vector<int> &fds() const { return m_fds; };

private:
    void parseArgs(int argc, char **argv);
    void usage();

    const char *m_pname;
    const char *m_cmd;
    const char **m_files;
    std::vector<int> m_fds;
};

}  // namespace imk

#endif /* __CONFIG_HPP__ */

