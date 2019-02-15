/*
 *  poll_bsd.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/event.h>
#include <sys/time.h>
#include <signal.h>

#include "../config.hpp"
#include "../poll_compat.hpp"
#include "../log.hpp"

using namespace std;
using namespace imk;

static int set_watch(int qfd, const char *path);
static void sigHandler(int);

//------------------------------------------------------------------------------------------------
int
Poll::watch(const string &path)
{
    if (m_qfd == -1) {
        signal(SIGINT, sigHandler);
        signal(SIGTERM, sigHandler);

        if ((m_qfd = kqueue()) == -1) {
            LOG_PERROR("kqueue");
        }
    }

    return set_watch(m_qfd, path.c_str());
}

//------------------------------------------------------------------------------------------------
int
Poll::dispatch()
{
    struct kevent ev;
    time_t next = { 0 };

    for (;;) {
        memset(&ev, 0, sizeof(ev));

        if (kevent(m_qfd, NULL, 0, &ev, 1, NULL) == -1) {
            if (errno == EINTR) {
                continue;
            }
            LOG_PERROR("kevent");
        }


        if (ev.flags & EVFILT_VNODE && ((ev.fflags & NOTE_DELETE) || (ev.fflags & NOTE_RENAME))) {
            usleep(m_cfg.sleepDelete * 1000);
        }

        int fd = (int)(intptr_t)ev.udata;
        const auto result = updateFd(fd);

        if (result.first) {
            LOG_INFO_VA("[====== %s (%u) =====]", result.second.c_str(), fd);

            if (time(NULL) > next) {
                runCommand(m_cfg.command);
                next = time(NULL) + m_cfg.threshold;
            }
        }
    }

    return 0;
}

//------------------------------------------------------------------------------------------------
void
Poll::close()
{
    ::close(m_qfd);

    for (const auto &fd : m_fds) {
        ::close(fd.first);
    }

    m_fds.clear();
}

//------------------------------------------------------------------------------------------------
int
set_watch(int qfd, const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        return fd;
    }

    struct kevent ev;
    int filter = EVFILT_VNODE;
    EV_SET(&ev, fd, filter, EV_ADD | EV_ONESHOT, NOTE_WRITE | NOTE_DELETE | NOTE_RENAME,
            0, (void*)(intptr_t)fd);

    if (kevent(qfd, &ev, 1, NULL , 0, NULL) == -1) {
        LOG_PERROR("kevent");
    }

    return fd;
}

//------------------------------------------------------------------------------------------------
void
sigHandler(int sig)
{
    LOG_ERR("interrupted");
    exit(15);
}

//------------------------------------------------------------------------------------------------
