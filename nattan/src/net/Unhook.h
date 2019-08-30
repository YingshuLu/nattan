/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_NET_UNHOOK_H
#define NATTAN_NET_UNHOOK_H

#include "base/Uncopyable.h"
extern "C" {
#include "deps/libcask/co_define.h"
}

namespace nattan {

class Unhook: public Uncopyable {
public:
    Unhook() {
        co_disable_hook();
    }

    ~Unhook() {
        co_enable_hook();
    }
};

} // namespace nattan

#endif
