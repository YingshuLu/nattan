/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATATN_UTIL_UUID_H
#define NATATN_UTIL_UUID_H

#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include "base/Uncopyable.h"
#include "base/Final.h"
#include "thread/Thread.h"

namespace nattan {

static volatile long LAST_TIME_STAMP = -1;
static volatile int SEQUENCE = 0;
#define SEQUENCE_MASK ((1<<13) - 1)

class UUID : public Final {

public:
    static long getTimeStamp() {
        struct timeval tv;
        ::gettimeofday(&tv, 0);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    static long get() {
        long current = getTimeStamp();
        if(current == LAST_TIME_STAMP) {
            SEQUENCE = ((++SEQUENCE) & SEQUENCE_MASK);
            if (SEQUENCE == 0) current = tillNextMillis(current);
        } else SEQUENCE = 0;
        LAST_TIME_STAMP = current;
        return (current << 22) | (tid() << 12) | SEQUENCE;
    }

private:
    static long tillNextMillis(long lastTimestamp) {
        long current = getTimeStamp();
        while(current <= lastTimestamp) {
            current = getTimeStamp();
        }
        return current;
    }

};

} // namespace nattan

#endif
