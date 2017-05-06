#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <stdio.h>
#include <string.h>

#define LOG_PERROR(M)           \
        fprintf(stderr, ":: [%s] ERROR: " M ": %s\n", get_time(), \
                strerror(errno));

#define LOG_ERR(M)              \
        fprintf(stderr, ":: [%s] ERROR: " M "\n", get_time());

#define LOG_ERR_VA(FMT, ...)    \
        fprintf(stderr, ":: [%s] ERROR: " FMT "\n", get_time(), __VA_ARGS__);

#define LOG_INFO(M)             \
        fprintf(stdout, ":: [%s] " M "\n", get_time());

#define LOG_INFO_VA(FMT, ...)   \
        fprintf(stdout, ":: [%s] " FMT "\n", get_time(), __VA_ARGS__);

#ifdef DEBUG
#define LOG_DEBUG(FMT, ...)     \
        fprintf(stderr, ":: [%s] DEBUG: " FMT "\n", get_time(), __VA_ARGS__);
#else
#define LOG_DEBUG(fmt, ...)
#endif

const char *get_time(void);

#endif /* __LOG_HPP__ */
