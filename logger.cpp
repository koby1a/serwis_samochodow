#include "logger.h"
#include <cstdarg>
#include <cstdio>
#include <string>
#include <fcntl.h>
#include <unistd.h>

static std::string g_path = "raport_symulacji.log";

/**
 * @brief Zapisuje cala tresc do deskryptora (best effort).
 */
static void serwis_write_all(int fd, const char* data, size_t len) {
    while (len > 0) {
        ssize_t w = write(fd, data, len);
        if (w <= 0) return;
        data += (size_t)w;
        len -= (size_t)w;
    }
}

void serwis_logger_set_file(const char* path) {
    if (path && path[0] != '\0') g_path = path;
}

void serwis_logger_reset_file() {
    unlink(g_path.c_str());
}

void serwis_log(const char* prefix, const char* msg) {
    if (!prefix) prefix = "log";
    if (!msg) msg = "";

    int fd = open(g_path.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0600);
    if (fd < 0) return;

    char buf[1400];
    int n = std::snprintf(buf, sizeof(buf), "[%s] %s\n", prefix, msg);
    if (n > 0) serwis_write_all(fd, buf, (size_t)n);

    close(fd);
}

void serwis_logf(const char* prefix, const char* fmt, ...) {
    if (!fmt) return;

    char msg[1024];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    serwis_log(prefix, msg);
}
