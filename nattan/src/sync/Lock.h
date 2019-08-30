/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_SYNC_LOCK_H
#define NATTAN_SYNC_LOCK_H

#include "base/Uncopyable.h"

namespace nattan {

class Lock: public Uncopyable {
public:
	virtual bool lock() = 0;
	virtual bool locked() = 0;
	virtual bool unlock() = 0;

protected:
	Lock() {}
	virtual ~Lock() {}
};

}

#endif
