#ifndef __IMK_HPP__
#define __IMK_HPP__

#include <vector>
#include <algorithm>

namespace imk {

class Config
{
public:
    Config() {}
    ~Config() {}

    void parseArgs(int argc, char **argv);

    const char *getCommand() const { return m_cmd; }
    const char **getFiles() const { return m_files; }

    void addFd(int fd) { m_fds.push_back(fd); }
    void setFd(int idx, int fd)
    {
        if (idx < 0 || idx >= m_fds.size()) { return; }
        m_fds[idx] = fd;
    }

    int getFdIndex(int fd)
    {
        std::vector<int>::const_iterator it =
            find(m_fds.begin(), m_fds.end(), fd);
        return (it == m_fds.end() ? -1 : (it - m_fds.begin()));
    }

    const std::vector<int> fds() const { return m_fds; };

private:
    const char *m_cmd;
    const char **m_files;
    std::vector<int> m_fds;
};

}  // namespace imk

#endif /* __IMK_HPP__ */

