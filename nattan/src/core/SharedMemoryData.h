/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_CORE_SHAREDMEMORYDATA_H
#define NATTAN_CORE_SHAREDMEMORYDATA_H

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include "core/MemoryData.h"

namespace nattan {

class SharedMemoryData {

public:
    SharedMemoryData(const size_t size): 
        fFileName(""), fSize(size), fAnonymous(true), fHandle(NULL), fPersist(false) {}

    SharedMemoryData(const char* filename, const size_t size): 
        fFileName(filename), fSize(size), fAnonymous(false), fHandle(NULL), fPersist(false) {}

    SharedMemoryData(const char* filename, const size_t size, bool persist, bool anony = false):
        fFileName(filename), fSize(size), fAnonymous(anony), fHandle(NULL), fPersist(persist) {}
         
    void* open() {
        bool exist = File::IsExisted(fFileName.data());
        int flags = 0;
        int fd = -1;
        off_t offset = 0;
        if (!fAnonymous) {
            if (exist) {
                fd == ::shm_open(fFileName.data(),O_RDWR, 0644);
            } else {
                fd = ::shm_open(fFileName.data(), O_RDWR | O_CREAT, 0644);
                ::ftruncate(fd, fSize);
            }
            if (fd < 0) return NULL;
        } else {
            flags |= MAP_ANONYMOUS;
        }
        
        fHandle = ::mmap(NULL, fSize, PROT_READ | PROT_WRITE, flags | MAP_SHARED, fd, offset);
        if (fHandle == MAP_FAILED) fHandle = NULL;
        ::close(fd);
        return fHandle;
    }

    ~SharedMemoryData() {
        ::munmap(fHandle, fSize);
        if (!fAnonymous && !fPersist)   ::shm_unlink(fFileName.data());
    }

public:
    std::string fFileName;
    size_t fSize;
    bool fAnonymous;
    void* fHandle;
    bool fPersist;
};

} // namespace nattan

#endif
