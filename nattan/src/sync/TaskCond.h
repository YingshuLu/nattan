/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_SYNC_TASKCOND_H
#define NATTAN_SYNC_TASKCOND_H

#include "sync/TaskMutex.h"
extern "C" {
#include "deps/libcask/co_cond.h"
}

namespace nattan {

class TaskCond {
public:
	TaskCond(TaskMutex& mux): tmutex(mux) {
		co_cond_init(&cond);	
	}

	void wait() {
		co_cond_wait(&cond, &(tmutex.mutex));
	}

	bool waitTill(time_t ms) {
		return co_cond_wait_till(&cond, &(tmutex.mutex), ms);
	}

	void signal() {
		co_cond_signal(&cond);
	}

	void signalAll() {
		co_cond_broadcast(&cond);
	}

private:
	TaskMutex& tmutex;
	co_cond_t cond;
};

} // namespace nattan

#endif
