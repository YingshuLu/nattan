/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_THREAD_FUTURE_H
#define NATTAN_THREAD_FUTURE_H

#include <memory>
#include "thread/ThreadMutex.h"
#include "thread/Condition.h"
#include "thread/Runnable.h"
#include "thread/Callable.h"

namespace nattan {

template<typename T>
class Future: public Runnable {

public:
    Future(std::shared_ptr<Callable<T>>& c): fCaller(c), fCond(fMutex) {}

    void run() {
        LockGuard guard(fMutex);
        fResult = fCaller->call();
        bFinished = true;
        fCond.notify();
    }   

    bool done() {
        return bFinished; 
    }

    T get() {
        if (!bFinished) {
            wait();
        }
        return fResult;
    }

    virtual ~Future() {}

private:
    void wait() {
        LockGuard guard(fMutex);
        while(!bFinished) {
            fCond.wait();
        }
    }

private:
    ThreadMutex fMutex;
    Condition fCond;
    volatile bool bFinished = false;

private:
    T fResult;
    std::shared_ptr<Callable<T>> fCaller;

};

} // namespace nattan

#endif
