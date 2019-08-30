/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_THREAD_THREADMUTEX_H
#define NATTAN_THREAD_THREADMUTEX_H

#include <stdio.h>
#include <sys/types.h>
#include <string>
#include "sync/Lock.h"
#include "thread/Thread.h"

namespace nattan {

class ThreadMutex: public Lock {
public:
    ThreadMutex()
        :holder_(0) {
        pthread_mutex_init(&mutex_, NULL);    
    }

    bool lock() {
        int ret = pthread_mutex_lock(&mutex_);
        if(ret == 0) {
            assignHolder();
        }
        return ret == 0;
    }

    bool try_lock() {
        int ret = pthread_mutex_trylock(&mutex_);
        if (ret == 0) {
            assignHolder();
        }
        return ret == 0;
    }

    bool locked() {
        return isLockedByCurrentThread();
    }

    bool unlock() {
        int ret = pthread_mutex_unlock(&mutex_);
        if (ret == 0) {
            unassignHolder();
        }
        return ret == 0;
    }

    int id() {
        return holder_;
    }
    
    bool isLockedByCurrentThread() const {
        return holder_ == tid();
    }

    ~ThreadMutex() {
        pthread_mutex_destroy(&mutex_);
    }

private:
    friend class Condition;

    pthread_mutex_t* getThreadMutex() {
        return &mutex_;
    }

    int init() {
        return pthread_mutex_init(&mutex_, NULL);
    }

    int destroy() {
        return pthread_mutex_destroy(&mutex_);
    }

    void assignHolder() {
        holder_ = tid();
    }

    void unassignHolder() {
        holder_ = 0;
    }
    
    class UnassignGuard {
    public:
        UnassignGuard(ThreadMutex& mutex)
        :owner_(mutex) {
            owner_.unassignHolder();
        }
        
        ~UnassignGuard() {
            owner_.assignHolder();
        }
    
    private:
        ThreadMutex& owner_;
    };

    pthread_mutex_t  mutex_;
    int holder_ = 0;
};

} // namespace nattan

#endif
