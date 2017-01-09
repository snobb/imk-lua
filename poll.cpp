/*
 *  poll.cpp
 *  author: Aleksei Kozadaev (2017)
 */

#include "poll_compat.hpp"

using namespace imk;

//------------------------------------------------------------------------------
Poll::Poll(Config &cfg) : m_cfg(cfg), m_qfd(-1)
{
    static int instance_cnt = 0;

    if (instance_cnt > 0) {
        LOG_ERR("Cannot have multiple instances");
        abort();
    }
    ++instance_cnt;

    m_fds.reserve(cfg.files.size());
    for (const auto &file : cfg.files) {
        m_fds.push_back(regFile(file));
    }
}

//------------------------------------------------------------------------------
ssize_t
Poll::getFdIndex(int fd)
{
    const auto &it = std::find(m_fds.begin(), m_fds.end(), fd);
    return (it == m_fds.end() ? -1 : (it - m_fds.begin()));;
}

//------------------------------------------------------------------------------
void
Poll::setFd(size_t idx, int fd)
{
    if (idx >= m_fds.size()) { return; }
    m_fds[idx] = fd;
}

//------------------------------------------------------------------------------
