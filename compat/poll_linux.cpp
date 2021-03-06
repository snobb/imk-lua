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

#include "../config.hpp"
#include "../poll_compat.hpp"
#include "../log.hpp"

using namespace std;
using namespace imk;

#define EVENT_SIZE      (sizeof(struct inotify_event))
#define BUF_LEN         (8 * (EVENT_SIZE + 16))

#define FILTERS         (IN_MODIFY | IN_ONESHOT | IN_DELETE_SELF | IN_MOVE_SELF)

static void sigHandler(int);

//------------------------------------------------------------------------------------------------
int
Poll::watch(const string &path)
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

//------------------------------------------------------------------------------------------------
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

            // sometimes IN_DELETE_SELF or IN_MOVE_SELF mean the file is being processed by some
            // program (eg. vim or gofmt), so if imk tries to reattach to the file immediately it
            // may not exist. So sleep for a while before try to reattach to the file.
            if ((ev->mask & IN_DELETE_SELF) || (ev->mask & IN_MOVE_SELF)) {
                usleep(m_cfg.sleepDelete * 1000);
            }

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
void
sigHandler(int sig)
{
    LOG_ERR("interrupted...");
    exit(15);
}

//------------------------------------------------------------------------------------------------
