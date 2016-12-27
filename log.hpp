#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <stdio.h>
#include <string.h>

#define LOG_PERROR(M)           \
        fprintf(stderr, ":: [%s] " M ": %s\n", get_time(), strerror(errno));

#define LOG_ERR(M)              \
        fprintf(stderr, ":: [%s] " M "\n", get_time());

#define LOG_INFO(M)             \
        fprintf(stdout, ":: [%s] " M "\n", get_time());

#define LOG_INFO_VA(M, ...)     \
        fprintf(stdout, ":: [%s] " M "\n", get_time(), __VA_ARGS__);

#define LOG_DEBUG(fmt, ...)     \
        fprintf(stderr, ":: [%s] " fmt "\n", get_time(), __VA_ARGS__);

const char *get_time(void);

#endif /* __LOG_HPP__ */
