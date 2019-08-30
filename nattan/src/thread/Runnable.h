/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_THREAD_RUNNABLE_H
#define NATTAN_THREAD_RUNNABLE_H

namespace nattan {

class Runnable {
public:
    virtual void run() = 0;

protected:
    virtual ~Runnable() {};
};

} // namespace nattan
#endif
