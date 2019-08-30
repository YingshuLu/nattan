/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_THREAD_THREADPOOL_H
#define NATTAN_THREAD_THREADPOOL_H

#include <memory>
#include <vector>
#include "thread/ArrayBlockingQueue.h"
#include "thread/Thread.h"
#include "thread/Runnable.h"

namespace nattan {

class ThreadPool {
friend class Worker;

private:
    class Worker: public Thread {
        public:
            Worker(ThreadPool& p): pool(p) {}
        protected:
            void run() {
                while(pool.running) {
                    std::shared_ptr<Runnable> r = pool.take();
                    r->run();
                }
            }
        private:
            ThreadPool& pool;
    };

    class EmptyRunner: public Runnable {
        public:
            void run() {
                return;
            }
    };

public:
    ThreadPool(const unsigned int& threadNumber = 2): threadNum(threadNumber) {}

    ThreadPool(const unsigned int& threadNumber, const unsigned int& queueCapcity = 64) :
        threadNum(threadNumber), tasks(queueCapcity) {}

    void start() {
        if (bStarted) return;
        bStarted = true;
        running = true;
        for (int i = 0; i < threadNum; i ++) {
            std::unique_ptr<Worker> wp(new Worker(*this));
            wp->start();
            workers.push_back(std::move(wp));
        }
    }

    bool serving() {
        return running;
    }

    void stop() {
        running = false;
        // avoid thread blocked on array
        for (int i = 0; i < threadNum; i++) {
            std::shared_ptr<Runnable> runner(new EmptyRunner());
            tasks.offer(runner);
        }

        for (auto iter = workers.begin(); iter != workers.end(); iter ++) {
            (*iter)->join();
        }
        workers.clear();
    }

    bool offer(std::shared_ptr<Runnable>& r) {
        if (!running) return false;
        return tasks.offer(r);
    }

    void put(std::shared_ptr<Runnable>& r) {
        if (!running) return;
        return tasks.put(r);
    }

    virtual ~ThreadPool(){
        stop(); 
    }

private:
    std::shared_ptr<Runnable> take() {
        return tasks.take();
    }

private:
    unsigned int threadNum;
    volatile bool running;
    bool bStarted = false;
    ArrayBlockingQueue<std::shared_ptr<Runnable> > tasks;
    std::vector<std::unique_ptr<Worker> > workers;
};

} // namespace nattan

#endif
