/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_THREAD_THREAD_H
#define NATTAN_THREAD_THREAD_H

#include <string>
#include <pthread.h>
#include "base/Uncopyable.h"
extern "C" {
#include "deps/libcask/cid.h"
}

namespace nattan {

void* _thread_run(void *p);

enum THREAD_STATE_T {
    THREAD_INIT,
    THREAD_RUNNING,
    THREAD_SLEEP,
    THREAD_DEAD
};

class Thread: public Uncopyable {
friend void* _thread_run(void *p);
public:
    void setName(const std::string& name) {
        threadName = name;
    }

    std::string name() {
        return threadName;
    }

    int start() {
        if (threadState < THREAD_RUNNING || threadState >= THREAD_DEAD) { 
            return ::pthread_create(&thread, NULL, _thread_run, this);
        }
        return -1;
    }

    int id() {
        return threadId;
    }

    static int gettid() {
        return tid();
    }

    THREAD_STATE_T state() {
        return threadState;
    }

    int detach() {
        if (detached) return 0;
        if (THREAD_RUNNING != threadState) return -1;
        int ret = ::pthread_detach(thread);
        if (ret == 0) detached = true;
        return ret;
    }

    int join() {
        if (THREAD_DEAD == threadState) return 0;
        return ::pthread_join(thread, NULL);
    }

    void exit() {
        if(THREAD_RUNNING == threadState) ::pthread_exit(NULL);
    }

    static Thread* currentThread(); 

protected:
    Thread(): threadId(-1), detached(false), threadState(THREAD_INIT) {}
    virtual void run() = 0;
    virtual ~Thread() {}

private:
    void setId() {
        threadId = gettid();
    }

private:
    int threadId;
    pthread_t thread;
    bool detached;
    volatile THREAD_STATE_T threadState;
    std::string threadName = "Thread";
};

} // namespace nattan

#endif
