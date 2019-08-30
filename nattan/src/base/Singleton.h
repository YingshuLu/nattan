/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_BASE_SINGLETON_H
#define NATTAN_BASE_SINGLETON_H

#include <memory>
#include "sync/SpinLock.h"
#include "sync/LockGuard.h"

namespace nattan {

template <typename T>
class Singleton {
public:
    static T*  getInstance() {
        if (!pInstance) {
            LockGuard guard(spinLock);
            if (!pInstance) {
                pInstance = std::shared_ptr<T>(new T());
            }
        }
        return pInstance.get();
    }

    static void destory() {
        LockGuard guard(spinLock);
        delete pInstance;
        pInstance = NULL;
    }

protected:
    Singleton() {}
    virtual ~Singleton() {}

private:
    static SpinLock spinLock;
    static std::shared_ptr<T> pInstance;
};

template <typename T>
std::shared_ptr<T> Singleton<T>::pInstance = NULL;

template <typename T>
SpinLock Singleton<T>::spinLock;

} // namespace nattan
#endif
