#ifndef __IMK_HPP__
#define __IMK_HPP__

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

    void addFd(int fd) { m_fds.push_back(fd); }
    void setFd(size_t idx, int fd)
    {
        if (idx >= m_fds.size()) { return; }
        m_fds[idx] = fd;
    }

    ssize_t getFdIndex(int fd)
    {
        std::vector<int>::const_iterator it =
            find(m_fds.begin(), m_fds.end(), fd);
        return (it == m_fds.end() ? -1 : (it - m_fds.begin()));
    }

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

#endif /* __IMK_HPP__ */

