/*
 *  poll.cpp
 *  author: Aleksei Kozadaev (2017)
 */

#include <string>

#include "poll_compat.hpp"

using namespace std;
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

    for (const auto &fileName : cfg.files) {
        m_fds[regFile(fileName)] = fileName;
    }
}

//------------------------------------------------------------------------------
std::pair<bool,string>
Poll::updateFd(int fd)
{
    const auto it = m_fds.find(fd);

    if (it != m_fds.end()) {
        const auto fileName = it->second;
        m_fds[regFile(fileName)] = fileName;
        m_fds.erase(it);
        return make_pair(true, fileName);
    } else {
        return make_pair(false, string());
    }
}

//------------------------------------------------------------------------------
void
Poll::runCommand(const string &cmd)
{
    int rv = system(cmd.c_str());
    if (rv < 0) {
        LOG_PERROR("===== system =====]");
    } else {
        LOG_INFO_VA("[====== %s (exit code %d) =====]",
                rv == 0 ? "ok" : "fail", rv);
    }

    if (m_cfg.oneRun) {
        ::exit(rv);
    }
}

//------------------------------------------------------------------------------
