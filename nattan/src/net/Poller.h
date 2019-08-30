/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_NET_POLLER_H
#define NATTAN_NET_POLLER_H

#include <poll.h>
extern "C" {
#include "deps/libcask/sys_helper.h"
#include "deps/libcask/co_define.h"
}

namespace nattan {

class Poller {

public:
    Poller() {
        co_disable_hook();
    }

    int operator()(struct pollfd* pfd, int n, time_t ms) {
        return co_poll(pfd, n, ms);
    }

    ~Poller() {
        co_enable_hook();
    }

};

} // namespace nattan

#endif
