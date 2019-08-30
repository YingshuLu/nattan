/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#include "thread/Thread.h"

namespace nattan {

__thread Thread* l_currentThread = NULL;

void* _thread_run(void *p) {
    if (p == NULL) return NULL;
    Thread *thread = (Thread *)p;
    thread->setId();
    thread->threadState = THREAD_RUNNING;
    l_currentThread = thread;
    thread->run();
    thread->threadState = THREAD_DEAD;
    l_currentThread = NULL;
    return NULL;
}

Thread* Thread::currentThread() {
    return l_currentThread;
}

} // namespace nattan
