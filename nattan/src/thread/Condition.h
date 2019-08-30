/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_THREAD_CONDITION_H
#define NATTAN_THREAD_CONDITION_H

#include <sys/types.h>
#include <time.h>
#include "base/Uncopyable.h"
#include "thread/ThreadMutex.h"

namespace nattan{
    
class Condition: public Uncopyable {
public:
    Condition(ThreadMutex& mutex)
        : mutex_(mutex) {
        pthread_cond_init(&cond_, NULL);
    }

    void wait() {
        ThreadMutex::UnassignGuard guard(mutex_);
        pthread_cond_wait(&cond_, mutex_.getThreadMutex());
    }

    bool waitForSeconds(double seconds) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);

        const int64_t kNanoSecondsPerSecond = 1e9;
        int64_t nanoseconds = static_cast<int64_t> (seconds * kNanoSecondsPerSecond);

        abstime.tv_sec += static_cast<time_t>(abstime.tv_sec + nanoseconds) / kNanoSecondsPerSecond;

        abstime.tv_nsec = static_cast<long> ((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

        ThreadMutex::UnassignGuard guard(mutex_);
        return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.getThreadMutex(), &abstime);
    }

    void notify() {
        pthread_cond_signal(&cond_);
    }

    void notifyAll() {
        pthread_cond_broadcast(&cond_);
    }

    ~Condition() {
        pthread_cond_destroy(&cond_);
    }

private:
    ThreadMutex& mutex_;
    pthread_cond_t cond_;
};

} // namespace nattan

#endif
