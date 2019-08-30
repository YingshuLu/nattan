/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#include "base/Singleton.h"
#include "thread/Runnable.h"
#include "thread/ThreadPool.h"
#include "task/Task.h"

extern "C" {
#include "deps/libcask/co_define.h"
#include "deps/libcask/co_inner_define.h"
}

namespace nattan {

void _task_run(void* ip, void* op) {
    if (NULL == ip) return;
    Task* t = (Task*) ip;
    t->handle = co_self();
    t->kId = Task::taskId();
    t->run();
    t->finish();
}

Task* Task::currentTask() {
    Task* t = NULL;
    if (co_self() != NULL) t = (Task*) (co_self()->context.iparam);
    return t;   
}

class AwaitRunner: public Runnable {
public:
    AwaitRunner(Runnable& r): fRunner(r), fRetCode(-1), fTask(NULL), fCond(fMutex), bFinished(false) {
        fTask = Task::currentTask();
    }

    void run() {
        fRunner.run();
        fRetCode = 0;
        if (NULL != fTask) {
            fTask->wakeup();
        } else { 
            notify();
        }
    }

    int result() {
        return fRetCode;
    }

private:
    friend int Task::await(Runnable& r);
    void wait() {
        if (!Task::inTask()) {
            LockGuard guard(fMutex);
            while(!bFinished) {
                fCond.wait();
            }
        }
    }

    void notify() {
        if (!Task::inTask()) {
            LockGuard guard(fMutex);
            bFinished = true;
            fCond.notify();
        }
    }

private:
    ThreadMutex fMutex;
    Condition fCond;
    Runnable& fRunner;
    Task* fTask;
    int fRetCode;
    volatile bool bFinished;
};

class AwaitThreadPoolSingleton: public Singleton<ThreadPool> {
public:
    AwaitThreadPoolSingleton() {
        ThreadPool* pool = getInstance();
        if (NULL != pool) pool->start();
    }   
};

int Task::await(Runnable& r) {
    AwaitThreadPoolSingleton awaitPool;
    ThreadPool* pool = awaitPool.getInstance();
    std::shared_ptr<Runnable> runner(new AwaitRunner(r));
    pool->put(runner);
    if (!Task::inTask()) {
        ((AwaitRunner*)(runner.get()))->wait();
    } else {
        co_sys_yield();
    }
    return ((AwaitRunner*)(runner.get()))->result();
}

} // namespace nattan
