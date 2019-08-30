/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_LOG_LOGGER_H
#define NATTAN_LOG_LOGGER_H

#include <string>
#include "base/Uncopyable.h"

namespace nattan {

enum LOGGER_LEVEL {
    LOGGER_ERROR,
    LOGGER_INFO,
    LOGGER_DEBUG
};

class Logger : public Uncopyable {

public:
    Logger(const int logfd);
    Logger(const std::string& logfile);
    void setLevel(LOGGER_LEVEL level);
    void printf(const char* fmt, ...); 
    void debug(const char* fmt, ...); 
    void info(const char* fmt, ...);
    void error(const char* fmt, ...);
    virtual ~Logger();

private:
    int fd;
    std::string logFile;
    LOGGER_LEVEL level = LOGGER_INFO;
};

} // namespace nattan

#endif
