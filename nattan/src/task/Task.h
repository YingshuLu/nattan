/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_TASK_TASK_H
#define NATTAN_TASK_TASK_H

#include "base/Uncopyable.h"
#include "thread/Runnable.h"
extern "C" {
#include "deps/libcask/co_define.h"
#include "deps/libcask/sys_helper.h"
}

namespace nattan {

void _task_run(void* ip, void* op);

class Task: public Uncopyable {
friend void _task_run(void *ip, void *op);
public:
    static bool inTask() {
        return co_self() != NULL;
    }

    int start() {
        co_create(_task_run, this, NULL);
        return 0;   
    }

    bool wakeup() {
        if(NULL == handle) return false;
        if (co_self() == handle) return true;

        sched_t* sched = *(handle->sched);
        sched->policy->enqueue(&sched->rq, handle);
        sched_active_event(sched);
        return true;
    }

    static Task* currentTask();

    static int sleep(unsigned long msecs) {
        return co_sleep(msecs);
    }

    static int lastErrno() {
        return co_errno();
    }

    int id() {
        return kId;
    }

    static int taskId() {
        return getcid();
    }

    static void yield() {
        co_yield();
    }

    static void exit() {
        co_exit();
    }

    static void sched() {
        schedule();
    }

    static int await(Runnable& runner);

protected:
    Task(){}
    virtual void run() = 0;

    void finish() {
        delete this;
    }

    virtual ~Task() {}
    task_t* handle = NULL;

private:
    int kId = -1;
};

} // namespace nattan

#endif
