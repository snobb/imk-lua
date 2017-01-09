/*
 *  poll_bsd.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/event.h>

#include "../imk.hpp"
#include "../poll_compat.hpp"
#include "../log.hpp"

using namespace std;
using namespace imk;

static int set_watch(int qfd, const char *path);
static void sig_handler(int);

static bool s_running = false;

//------------------------------------------------------------------------------
int
Poll::regFile(const char *path)
{
    if (m_qfd == -1) {
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);

        if ((m_qfd = kqueue()) == -1) {
            LOG_PERROR("kqueue");
        }
    }

    return set_watch(m_qfd, path);
}

//------------------------------------------------------------------------------
int
Poll::dispatch()
{
    struct kevent ev;

    s_running = true;
    while (s_running) {
        memset(&ev, 0, sizeof(ev));

        if (kevent(m_qfd, NULL, 0, &ev, 1, NULL) == -1) {
            if (errno == EINTR) {
                continue;
            }
            LOG_PERROR("kevent");
        }

        int idx = getFdIndex((int)(intptr_t)ev.udata);
        if (idx != -1) {
            int fd = set_watch(m_qfd, m_cfg.files[idx]);
            setFd(idx, fd);

            LOG_INFO_VA("[====== %s (%u) =====]", m_cfg.files[idx], fd);
            system(m_cfg.command.c_str());
        }
    }

    return 0;
}

//------------------------------------------------------------------------------
void
Poll::close()
{
    ::close(m_qfd);
    for (const auto &fd : m_fds) {
        ::close(fd);
    }
    m_fds.clear();
}

//------------------------------------------------------------------------------
int
set_watch(int qfd, const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        return fd;
    }

    struct kevent ev;
    int filter = EVFILT_VNODE;
    EV_SET(&ev, fd, filter, EV_ADD | EV_ONESHOT,
            NOTE_WRITE | NOTE_DELETE, 0, (void*)(intptr_t)fd);

    if (kevent(qfd, &ev, 1, NULL , 0, NULL) == -1) {
        LOG_PERROR("kevent");
    }

    return fd;
}

//------------------------------------------------------------------------------
void
sig_handler(int sig)
{
    s_running = false;
    LOG_ERR("interrupted");
}

//------------------------------------------------------------------------------
