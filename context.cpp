/*
 *  context.cpp
 *  author: Aleksei Kozadaev (2017)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/inotify.h>

#include "context.hpp"

using namespace imk;
using namespace std;

#define EVENT_SIZE      (sizeof(struct inotify_event))
#define BUF_LEN         (1024 * (EVENT_SIZE + 16))
#define FILTERS         (IN_ALL_EVENTS)

static void sig_handler(int);

static bool s_running = false;

//------------------------------------------------------------------------------
Context::Context() : m_qfd(-1)
{
    static int instCount = 0;

    if (instCount > 0) {
        LOG_ERR("Cannot have multiple instances");
        abort();
    }
    ++instCount;

    char rcFile[FILENAME_MAX] = { 0 };
    if (findRcFile(rcFile) != -1) {
        m_luaInterp.executeScript(rcFile);
    }

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    if ((m_qfd = ::inotify_init()) == -1) {
        LOG_PERROR("inotify_init");
        exit(1);
    }
}

//------------------------------------------------------------------------------
Context::~Context()
{
    close(m_qfd);
    for (auto it = files().cbegin(); it != files().cend(); ++it) {
        close(it->first);
    }
}

//------------------------------------------------------------------------------
int
Context::walkDir(const char *path)
{
    char name[FILENAME_MAX];
    struct dirent *dp;
    DIR *dir;
    size_t dirLen;

    if ((dir = opendir(path)) == NULL) {
        goto error;
    }

    dirLen = strlen(path);
    while ((dp = readdir(dir)) != NULL) {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
            continue;
        }

        if ((dirLen + strlen(dp->d_name)+2) > FILENAME_MAX) {
            goto error;
        }
        else {
            if (dp->d_type == DT_REG || dp->d_type == DT_DIR) {
                sprintf(name, "%s/%s", path, dp->d_name);
                registerFile(name);
            }
        }
    }
    closedir(dir);
    return 0;

error:
    if (dir) { closedir(dir); }
    LOG_PERROR("");
    return -1;
}

//------------------------------------------------------------------------------
int
Context::findRcFile(char *rcFileOut)
{
    char *homeDir;

    snprintf(rcFileOut, FILENAME_MAX, "%s", RC_FILE);
    if (access(rcFileOut, F_OK) != -1) {
        LOG_DEBUG("found lua config %s", rcFileOut);
        return 0;
    }

    if ((homeDir = getenv("HOME")) == NULL) {
        homeDir = getpwuid(getuid())->pw_dir;
    }

    snprintf(rcFileOut, FILENAME_MAX, "%s/%s", homeDir, RC_FILE);
    if (access(rcFileOut, F_OK) != -1) {
        LOG_DEBUG("found lua config %s", rcFileOut);
        return 0;
    }

    return -1;
}

//------------------------------------------------------------------------------
void
Context::registerFile(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        LOG_DEBUG("file %s does not exist", path);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        // recursively add all files in the directory
        walkDir(path);
    }
    else {
        struct stat st;
        if (stat(path, &st) != 0) {
            return;
        }

        int fd = inotify_add_watch(m_qfd, path, FILTERS);
        if (fd == -1) {
            LOG_PERROR("inotify_add_watch");
            exit(1);
        }

        m_files[fd] = path;
    }
}

//------------------------------------------------------------------------------
int
Context::dispatch()
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

        bool need_cmd = false;
        for (ssize_t i = 0; i < len;) {
            struct inotify_event *ev =
                reinterpret_cast<struct inotify_event*>(&buf[i]);

            const auto it = m_files.find(ev->wd);
            if (it == m_files.cend()) {
                break;
            }

            const string fname = it->second;

            if (ev->mask & IN_OPEN) {
                LOG_DEBUG("trigger open cb for %s", fname.c_str());
                m_luaInterp.eventOpen(fname.c_str());
            }
            else if (ev->mask & IN_CLOSE) {
                LOG_DEBUG("trigger close cb for %s", fname.c_str());
                m_luaInterp.eventClose(fname.c_str());
            }
            else if (ev->mask & IN_CREATE) {
                LOG_DEBUG("trigger create cb for %s", fname.c_str());
                m_luaInterp.eventCreate(fname.c_str());
            }
            else if (ev->mask & IN_ACCESS) {
                LOG_DEBUG("trigger read cb for %s", fname.c_str());
                m_luaInterp.eventRead(fname.c_str());
            }
            else if ((ev->mask & IN_MODIFY) || (ev->mask & IN_ATTRIB)) {
                LOG_DEBUG("trigger write cb for %s", fname.c_str());
                LOG_INFO_VA("[ ====== event %s (%u) =====]", fname.c_str(), ev->wd);
                if (m_luaInterp.eventWrite(fname.c_str()) != LUA_OK) {
                    need_cmd = true;
                }
            }
            else if (ev->mask & IN_MOVE_SELF) {
                LOG_DEBUG("move_self event for %s", fname.c_str());
                int wd = inotify_add_watch(m_qfd, fname.c_str(), FILTERS);
                if (wd == -1) {
                    LOG_PERROR("inotify_add_watch");
                }
                m_files.erase(ev->wd);
                m_files[wd] = fname;
            }
            else if ((ev->mask & IN_DELETE) || (ev->mask & IN_DELETE_SELF)) {
                LOG_DEBUG("trigger delete cb for %s", fname.c_str());
                m_luaInterp.eventDelete(fname.c_str());
                if (ev->mask & IN_DELETE_SELF) {
                    m_files.erase(ev->wd);
                }
            }
            else if (ev->mask & IN_IGNORED) {
                LOG_DEBUG("IGNORED event for %s", fname.c_str());
            }

            i += EVENT_SIZE + ev->len;
        }

        if (need_cmd && !m_command.empty()) {
            system(command().c_str());
            need_cmd = false;
        }
    }

    return 0;
}

//------------------------------------------------------------------------------
// static methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void
sig_handler(int sig)
{
    s_running = false;
    LOG_ERR("interrupted");
}

//------------------------------------------------------------------------------
