/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_TASK_TASKTHREAD_H
#define NATTAN_TASK_TASKTHREAD_H

#include <iostream>
#include <memory>
#include <vector>
#include <set>
#include "thread/Runnable.h"
#include "thread/Thread.h"
#include "thread/ArrayBlockingQueue.h"
#include "task/Task.h"
#include "sync/Chan.h"
#include "log/Log.h"

extern "C" {
#include "deps/libcask/task_sched.h"
}

namespace nattan {

class TaskThread: public Thread {

class TaskWorker: public Task {
public:
    TaskWorker(TaskThread& thread, const unsigned int chan_size = 1): fThread(thread), fChan(chan_size){
        fRunning = true;
    }

    void send(std::shared_ptr<Runnable>& runner) {
        fChan.send(runner);
    }

    bool submitTask(std::shared_ptr<Runnable>& runner) {
        return fThread.submit(runner);
    }

    void stop() {
        fRunning = false;
    }
    
    virtual ~TaskWorker() {}

protected:
    virtual void run() {
        long timeout = TaskThread::currentTaskThread()->getKeepaliveTimeout();
        std::shared_ptr<Runnable> runner;
        while(fRunning) {
            if (timeout > 0) {
                // keepalive timeout happend
                if (!fChan.recvTill(runner, timeout)) { 
                    DBG_LOG("NOTE: task keepalive timeout, go die");
                    TaskThread::currentTaskThread()->removeWorker(this);
                    break; 
                }
            } else {
                runner = fChan.recv();
            }
            runner->run();
            rest();    
        }
    }

    std::shared_ptr<Runnable> recv() {
        return fChan.recv();
    }

    bool fRunning;
    TaskThread& fThread;

private:
    void rest() {
        fThread.easeWorker(this);
    }

    Chan<std::shared_ptr<Runnable>> fChan;

};

class TaskBoss: public TaskWorker {
public:
    TaskBoss(TaskThread& thread, const unsigned int chan_size = 64): TaskWorker(thread, chan_size) {}

    virtual ~TaskBoss() {}
protected:
    void run() {
        TaskWorker* task = NULL;
        bool bPolled = false;
        std::shared_ptr<Runnable> runner = nullptr;
        while(fRunning) {
            //consume thread task first
            bPolled = fThread.fTasks.poll(runner);
            if (!bPolled) {
                runner = recv();
            } 

            if (fThread.fIdleWorkers.empty()) {
                task = new TaskWorker(fThread);
                task->start();
            } else {
                auto it = fThread.fIdleWorkers.begin();
                task = *it;
                fThread.fIdleWorkers.erase(it);
            }
            task->send(runner);
        }
    }
};

friend class TaskBoss;

public:
    TaskThread(const long timeout = -1): fTimeout(timeout) {}

    bool submit(std::shared_ptr<Runnable>& runner) {
        //avoid explore 
        if (state() != THREAD_RUNNING) return false;
        if(Task::inTask()) {
            fBoss->send(runner);
            activeSched();
        } else {
            return fTasks.offer(runner);    
        }
        return true;
    }

    static TaskThread* currentTaskThread() {
        return (TaskThread*)Thread::currentThread();
    }

    long getKeepaliveTimeout() {
        return fTimeout;
    }

protected:
    void run() {
        fBoss = new TaskBoss(*this, 128);
        fBoss->start();
        fSched = current_sched();
        Task::sched();
    }

    void activeSched() {
        sched_active_event(fSched); 
    }

private:
    void easeWorker(TaskWorker* worker) {
        if (worker != NULL) fIdleWorkers.insert(worker);
    }
    
    void removeWorker(TaskWorker* worker) {
        if (worker != NULL) fIdleWorkers.erase(worker);
    }

private:
    std::set<TaskWorker*> fIdleWorkers;
    ArrayBlockingQueue<std::shared_ptr<Runnable>> fTasks;
    TaskBoss* fBoss = nullptr;
    sched_t* fSched = nullptr;
    long fTimeout = -1;

};

} //namespace nattan

#endif
