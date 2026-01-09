// logger.cpp
#include "logger.h"
#include <fstream>
#include <string>
#include <cstdarg>
#include <cstdio>

static std::string g_log_path = "raport_symulacji.log";

void serwis_logger_set_file(const char* path) {
    if (path && path[0] != '\0') {
        g_log_path = path;
    }
}

void serwis_log(const char* prefix, const char* msg) {
    std::ofstream f(g_log_path.c_str(), std::ios::app);
    if (!f.is_open()) return;

    if (!prefix) prefix = "log";
    if (!msg) msg = "";

    f << "[" << prefix << "] " << msg << "\n";
}

void serwis_logf(const char* prefix, const char* fmt, ...) {
    if (!fmt) return;

    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    serwis_log(prefix, buf);
}
