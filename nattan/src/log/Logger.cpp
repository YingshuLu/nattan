/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <unistd.h>
#include <string>
#include "log/Logger.h"
#include "log/Log.h"
#include "sync/LockGuard.h"
extern "C" {
#include "deps/libcask/log.h"
}

namespace nattan {

bool DevelopLog::kSwitchOn = false;

static const unsigned int LOG_BUFFER_SIZE = 4096;

Logger::Logger(const int logfd): fd(logfd){}

Logger::Logger(const std::string& logfile): logFile(logfile) {
    fd = ::open(logFile.data(), O_WRONLY | O_CREAT | O_APPEND, 0666);
}

void Logger::setLevel(LOGGER_LEVEL level) {
    this->level = level;
}

void Logger::printf(const char* fmt, ...) {
    char buffer[LOG_BUFFER_SIZE] = {0};
    int n = 0;
    va_list args;
    va_start(args, fmt);
    n = vsnprintf(buffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);
    if (n > 0 && n < LOG_BUFFER_SIZE) {
        buffer[n] = '\n';
    }
    LockGuard guard(lock);
    log_printf(fd, LOG_INFO, "%s", buffer);

}

void Logger::debug(const char* fmt, ...) {
    if (level < LOGGER_DEBUG) return;
    char buffer[LOG_BUFFER_SIZE] = {0};
    int n = 0;
    va_list args;
    va_start(args, fmt);
    n = vsnprintf(buffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    if (n > 0 && n < LOG_BUFFER_SIZE) {
        buffer[n] = '\n';
    }
    LockGuard guard(lock);
    log_printf(fd, LOG_DEBUG, "%s", buffer);
}

void Logger::info(const char* fmt, ...) {
    if (level < LOGGER_INFO) return;
    char buffer[LOG_BUFFER_SIZE] = {0};
    int n = 0;
    va_list args;
    va_start(args, fmt);
    n = vsnprintf(buffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);
    if (n > 0 && n < LOG_BUFFER_SIZE) {
        buffer[n] = '\n';
    }
    LockGuard guard(lock);
    log_printf(fd, LOG_INFO, "%s", buffer);
}

void Logger::error(const char* fmt, ...) {
    if (level < LOGGER_ERROR) return;
    char buffer[LOG_BUFFER_SIZE] = {0};
    int n = 0;
    va_list args;
    va_start(args, fmt);
    n = vsnprintf(buffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);
    if (n > 0 && n < LOG_BUFFER_SIZE) {
        buffer[n] = '\n';
    }
    LockGuard guard(lock);
    log_printf(fd, LOG_ERROR, "%s", buffer);
}

Logger::~Logger() {
    ::fdatasync(fd);
    ::close(fd);
}

} // namespace nattan
