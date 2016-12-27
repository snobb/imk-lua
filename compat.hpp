#ifndef __POLL_HPP__
#define __POLL_HPP__

#include <stdlib.h>
#include <errno.h>

#include "log.hpp"

namespace imk {

class Poll
{
public:
    Poll(Config &cfg) : m_cfg(cfg), m_qfd(-1)
    {
        static int instance_cnt = 0;

        if (instance_cnt > 0) {
            LOG_ERR("Cannot have multiple instances");
            abort();
        }
        ++instance_cnt;
    }

    ~Poll() { close(); }

    int regFile(const char *path);
    int dispatch();
    void close();

private:
    Config &m_cfg;
    int m_qfd;
};

}  // namespace imk

#endif /* __POLL_HPP__ */
