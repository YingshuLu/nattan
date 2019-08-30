/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_SYNC_SPINLOCK_H
#define NATTAN_SYNC_SPINLOCK_H

#include "base/Uncopyable.h"
#include "sync/Lock.h"

extern "C" {
#include "deps/libcask/co_spin_lock.h"
#include "deps/libcask/co_define.h"
}

namespace nattan {

class SpinLock: public Lock {
public:
	SpinLock() {
		spin_lock_init(&spinLock);
	}

	bool lock() {
		spin_lock(&spinLock);
		return true;
	}

	bool locked() {
		return spin_locked(&spinLock);
	}
	
	bool unlock() {
		spin_unlock(&spinLock);
		return true;
	}

private:
	spin_lock_t spinLock;

};

}

#endif
