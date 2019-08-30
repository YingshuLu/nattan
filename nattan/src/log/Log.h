/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_LOG_DEVELOP_LOG_H
#define NATTAN_LOG_DEVELOP_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "base/Final.h"

extern "C" {
#include "deps/libcask/types.h"
}

namespace nattan {

class DevelopLog: public Final {

public:
    static void setDebugFile(const char* file) {
        ::close(LOG_FD_STDOUT);     
        const char *log_file = kSwitchOn? file : "/dev/null";
        int fd = ::open(log_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    }

    static void setErrorFile(const char* file) {
        ::close(LOG_FD_STDERR);     
        const char *log_file = kSwitchOn? file : "/dev/null";
        int fd = ::open(log_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    }

    static void switchOn() {
        kSwitchOn = true;
    }

private:
    static bool kSwitchOn;
};

} // namespace nattan

#endif
