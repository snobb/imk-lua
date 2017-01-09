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
#define BUF_LEN         (1024 * (EVENT_SIZE + 16))

#define FILTERS         (IN_MODIFY | IN_ONESHOT)

static bool s_running = false;

static void sig_handler(int);

//------------------------------------------------------------------------------
int
Poll::regFile(const char *path)
{
    int rv;

    if (m_qfd == -1) {
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);

        if ((m_qfd = ::inotify_init()) == -1) {
            LOG_PERROR("inotify_init");
            exit(1);
        }
    }

    struct stat st;
    if ((rv = stat(path, &st)) != 0) {
        return -1;
    }

    rv = inotify_add_watch(m_qfd, path, FILTERS);

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

    s_running = true;
    while (s_running) {
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

            ssize_t idx = getFdIndex(ev->wd);
            if (idx != -1) {
                LOG_INFO_VA("[====== %s (%u) =====]", m_cfg.files[idx], ev->wd);
                int wd = inotify_add_watch(m_qfd, m_cfg.files[idx], FILTERS);
                if (wd == -1) {
                    LOG_PERROR("inotify_add_watch");
                }
                setFd(idx, wd);
            }

            i += EVENT_SIZE + ev->len;
        }

        system(m_cfg.command.c_str());
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
void
sig_handler(int sig)
{
    s_running = false;
    LOG_ERR("interrupted");
}

//------------------------------------------------------------------------------
