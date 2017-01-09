#ifndef __POLL_COMPAT_HPP__
#define __POLL_COMPAT_HPP__

#include <stdlib.h>
#include <errno.h>
#include <algorithm>

#include "imk.hpp"
#include "log.hpp"

namespace imk {

class Poll
{
public:
    Poll(Config &cfg);
    ~Poll() { close(); }

    // Platform depenedant methods defined in /compat
    int regFile(const std::string &path);
    int dispatch();

private:
    ssize_t getFdIndex(int fd);
    void setFd(size_t idx, int fd);
    void close();

    Config &m_cfg;
    int m_qfd;
    std::vector<int> m_fds;

    // disable default and copy constructors;
    Poll() =delete;
    Poll(Poll &) = delete;
    Poll(const Poll &) = delete;
};

}  // namespace imk

#endif /* __POLL_COMPAT_HPP__ */
