/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_THREAD_CALLABLE_H
#define NATTAN_THREAD_CALLABLE_H

namespace nattan {

template<typename T>
class Callable {
public:
    virtual T call() = 0;
    virtual ~Callable(){}
};

} // namespace nattan

#endif
