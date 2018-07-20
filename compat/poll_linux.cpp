/*
 *  poll_linux.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/inotify.h>

#include "../imk.hpp"
#include "../poll_compat.hpp"
#include "../log.hpp"

using namespace std;
using namespace imk;

#define EVENT_SIZE      (sizeof(struct inotify_event))
#define BUF_LEN         (8 * (EVENT_SIZE + 16))

#ifdef VBOX
#define FILTERS         (IN_MOVE_SELF | IN_ONESHOT)
#else
#define FILTERS         (IN_MODIFY | IN_ONESHOT)
#endif

static void sigHandler(int);

//------------------------------------------------------------------------------
int
Poll::regFile(const string &path)
{
    int rv;

    if (m_qfd == -1) {
        signal(SIGINT, sigHandler);
        signal(SIGTERM, sigHandler);

        if ((m_qfd = ::inotify_init()) == -1) {
            LOG_PERROR("inotify_init");
            exit(1);
        }
    }

    struct stat st;
    if ((rv = stat(path.c_str(), &st)) != 0) {
        return -1;
    }

    rv = inotify_add_watch(m_qfd, path.c_str(), FILTERS);

    if (rv == -1) {
        LOG_PERROR("inotify_add_watch");
        exit(1);
    }

    return rv;
}

//------------------------------------------------------------------------------
int
Poll::dispatch()
{
    ssize_t len;
    char buf[BUF_LEN];
    time_t next = { 0 };

    for (;;) {
        if ((len = read(m_qfd, buf, BUF_LEN)) == -1) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            LOG_PERROR("read");
            exit(1);
        }

        if (len == 0) {
            continue;
        }

        for (ssize_t i = 0; i < len;) {
            struct inotify_event *ev = (struct inotify_event *)&buf[i];

            const auto result = updateFd(ev->wd);
            if (result.first) {
                LOG_INFO_VA("[====== %s (%u) =====]", result.second.c_str(), ev->wd);
            }

            i += EVENT_SIZE + ev->len;
        }

        if (time(NULL) > next) {
            runCommand(m_cfg.command);
            next = time(NULL) + m_cfg.threshold;
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
        ::close(fd.first);
    }
    m_fds.clear();
}

//------------------------------------------------------------------------------
void
sigHandler(int sig)
{
    LOG_ERR("interrupted...");
    exit(15);
}

//------------------------------------------------------------------------------
