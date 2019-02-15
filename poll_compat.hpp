#ifndef __POLL_COMPAT_HPP__
#define __POLL_COMPAT_HPP__

#include <stdlib.h>
#include <errno.h>
#include <algorithm>
#include <map>

#include "config.hpp"
#include "log.hpp"


namespace imk {

class Poll
{
public:
    Poll(Config &cfg);
    ~Poll() { close(); }

    // platform specific
    int dispatch();

private:
    // platform specific
    int watch(const std::string &path);
    void runCommand(const std::string &cmd);
    void close();

    // generic
    std::pair<bool,std::string> updateFd(int fd);

    const Config &m_cfg;
    int m_qfd;
    std::map<int, std::string> m_fds;

    // disable default and copy constructors;
    Poll() = delete;
    Poll(Poll &) = delete;
    Poll(const Poll &) = delete;
};

}  // namespace imk

#endif /* __POLL_COMPAT_HPP__ */
