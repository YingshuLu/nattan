/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_THREAD_ARRAY_BLOCKING_QUEUE_H
#define NATTAN_THREAD_ARRAY_BLOCKING_QUEUE_H

#include <memory>
#include "thread/Condition.h"
#include "thread/ThreadMutex.h"
#include "sync/LockGuard.h"

namespace nattan {

template <typename T>
class ArrayBlockingQueue {

public:
    #define DEFAULT_CAPCITY 64
    typedef unsigned int size_type;

    ArrayBlockingQueue(const size_type capcity = DEFAULT_CAPCITY):
        notFull_(lock_), notEmpty_(lock_), capcity_(capcity) {
        items_= std::unique_ptr<T[]>(new T[capcity]);
    }

    ~ArrayBlockingQueue() {
    }

    size_type size() { return count_; }

    bool isFull() { return count_ == capcity_; }

    bool isEmpty() { return count_ == 0; }

    bool offer(T& e) {
        if(isFull()) return false;
        if(!lock_.try_lock()) return false;
        if(isFull()) return false;
        enqueue(e);
        lock_.unlock();
        return true;
    }

    void put(T& e) {
        LockGuard guard(lock_);
        while(isFull()) {
            notFull_.wait();
        }
        enqueue(e);
    }

    bool poll(T& o) {
        if(isEmpty()) return false;
        if(!lock_.try_lock()) return false;
        if(isEmpty()) return false;
        o = dequeue();
        lock_.unlock();
        return true;
    }

    T take() {
        LockGuard guard(lock_);
        while(isEmpty()) {
            notEmpty_.wait();
        }
        return dequeue();    
    }
    
private:
    void enqueue(T& e) {
       items_[putIndex_] = e;
       if(++putIndex_ == capcity_) putIndex_ = 0;
       count_++;
       notEmpty_.notifyAll();
       /*if (count_ > 1) notEmpty_.notifyAll();
       else  notEmpty_.notify();*/
    }

    T dequeue() {
        T& e = items_[takeIndex_];
        if(++takeIndex_ == capcity_) takeIndex_ = 0;
        count_--;
        notFull_.notifyAll();
        /*
        if (count_ < capcity_ - 1) notFull_.notifyAll();
        else notFull_.notify();
        */
        return e;
    }
  
private:
    std::unique_ptr<T[]> items_;
    ThreadMutex lock_;
    Condition notFull_;
    Condition notEmpty_;

    volatile size_type count_ = 0;
    const size_type capcity_;
    volatile size_type putIndex_ = 0;
    volatile size_type takeIndex_ = 0;
};

} //namespace nattan

#endif
