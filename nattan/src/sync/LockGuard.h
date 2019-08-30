/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_SYNC_LOCKGUARD_H
#define NATTAN_SYNC_LOCKGUARD_H

#include "sync/Lock.h"
#include "base/Uncopyable.h"

namespace nattan {

class LockGuard: public Uncopyable {
public:
	LockGuard(Lock& lc): lock(lc) {
		lock.lock();
	}

	~LockGuard() {
		lock.unlock();
	}

private:
	Lock& lock;
};

}

#endif
