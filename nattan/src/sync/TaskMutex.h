/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_SYNC_TASKMUTEX_H
#define NATTAN_SYNC_TASKMUTEX_H

#include "sync/Lock.h"
extern "C" {
#include "deps/libcask/co_mutex.h"
}

namespace nattan {

class TaskMutex: public Lock {
friend class TaskCond;
public:
	TaskMutex() {
		co_mutex_init(&mutex);
	}

	bool lock() {
		co_mutex_lock(&mutex);
		return true;
	}

	bool locked() {
		return co_mutex_locked(&mutex);
	}	

	bool tryLock() {
		return co_mutex_try_lock(&mutex);
	}

	bool unlock() {
		co_mutex_unlock(&mutex);
		return true;
	}

private:
	co_mutex_t mutex;
};

}

#endif
