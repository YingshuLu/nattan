/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_SYNC_TIMER_H
#define NATTAN_SYNC_TIMER_H

#include <sys/timerfd.h>
#include <memory>
#include "util/UUID.h"

extern "C" {
#include "deps/libcask/timer.h"
}

namespace nattan {

class Timer {
public:

	Timer() {}

    int open() {
        if (fTimerFd < 0) {
		    fTimerFd = open_timer();
        }
        return 0;
    }

    static time_t CurrentTimeMs() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (time_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
    }

	bool set(time_t ms) {
		return set_timer(fTimerFd, ms) >= 0;
	}

	bool cancel() {
		return cancel_timer(fTimerFd) >= 0;	
	}

	bool wait(time_t ms) {
		return wait_timer(fTimerFd, ms) >= 0;
	}

	bool timeout() {
		return is_timeout_timer(fTimerFd) != 0;
	}

    void handle() {
        handle_timer(fTimerFd);
    }

	void close() {
		close_timer(fTimerFd);
		fTimerFd = -1;
	}

	~Timer() {
		if (fTimerFd > 0) close();
	}

private:
	int fTimerFd = -1;

};

} //namespace nattan

#endif

