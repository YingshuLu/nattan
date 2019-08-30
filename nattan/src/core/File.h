/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_CORE_FILE_H
#define NATTAN_CORE_FILE_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <string>
#include <algorithm>
#include "core/Data.h"
#include "base/Writeable.h"
#include "sync/Lock.h"
#include "base/Uncopyable.h"

namespace nattan {

const std::string FILE_DEFAULT_DIR = "/tmp/";
#define TEST_FILEACCESS(filename, mode) access(filename, mode) == 0;
static const size_t kFileBufferSize = 4096;

class File: public Data, public Lock {

public:
    File(const std::string& fileName): fFullName(fileName), fReadFd(-1), fWriteFd(-1), fLockFd(-1), fLocked(false) {}

    bool readable() { return TEST_FILEACCESS(fFullName.c_str(), R_OK); }
    bool writeable() { return TEST_FILEACCESS(fFullName.c_str(), W_OK); }
    bool executeable() { return TEST_FILEACCESS(fFullName.c_str(), X_OK); }
    bool existed() { return TEST_FILEACCESS(fFullName.c_str(), F_OK); }

    static bool IsExisted(const std::string& filename) {
        return TEST_FILEACCESS(filename.data(), F_OK);
    }
    
    static std::string getCurrentDir() {
        char cwd[1024] = {0};
        getcwd(cwd, 1024);
        std::string cwd_str = cwd;
        return cwd_str;
    }

    size_t length() {
        if (!existed()) return 0;
        int fd = ::open(fFullName.data(), O_RDONLY);
        ssize_t offset = ::lseek(fd, 0L, SEEK_END);
        if (offset < 0) offset = 0;
        ::close(fd);
        return offset;
    }

    const char* name() {
        return fFullName.data();
    }

    bool openRead(const ssize_t offset = -1) {
        if (fReadFd > 0 && offset == -1) return true;
        if (fReadFd <= 0) {
            fReadFd = ::open(fFullName.data(), O_RDONLY | O_CREAT, 0644);
            if (fReadFd <= 0) return false;
        }

        if (offset >= 0) {
            if (::lseek(fReadFd, offset, SEEK_SET) < 0) {
                closeRead();
                return false;
            }
        }
        return true;
    }

    bool openWrite() {
        if (fWriteFd > 0) return true;
        fWriteFd = ::open(fFullName.data(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fWriteFd <= 0) return false;
        return true;
    }

    bool openLock() {
        if (fLockFd > 0) return true;
        fLockFd = ::open(fFullName.data(), O_RDWR | O_CREAT, 0644);
        if (fLockFd <= 0) return false;
        return true;
    }

    bool lock() {
        bool ret = (::flock(fLockFd, LOCK_EX) == 0);
        if (ret) fLocked = true;
        return ret;
    }

    bool locked() { return fLocked; }

    bool tryLock() {
        bool ret = (::flock(fLockFd, LOCK_EX | LOCK_NB) == 0);
        if (ret) fLocked = true;
        return false;
    }

    bool unlock() {
        bool ret = (::flock(fLockFd, LOCK_UN) == 0);
        if (ret) fLocked = false;
        return ret;
    }

    ssize_t setLabelLength(const ssize_t len) {
        fLabelLength = len;
        return fLabelLength;
    }

    ssize_t read(char* buf, size_t buf_len) {
        openRead();
        return ::read(fReadFd, buf, buf_len);
    }

    ssize_t readRange(char *buf, size_t buf_len, size_t offset, size_t len) {
        openRead();
        int fd = fReadFd;
        int rs = ::lseek(fd, offset, SEEK_SET);
        if (rs < 0) {
            return rs;
        }
        size_t rlen = std::min(buf_len, len);
        rs = ::read(fd, buf, rlen);
        return rs;
    }

    ssize_t sendTo(Writeable& w) {
        if (fLabelLength > 0) {
            return sendTo(w, fLabelLength);
        }

        char buf[kFileBufferSize] = {0};
        int cnt = 0;
        int len = 0;
        while((cnt = read(buf, kFileBufferSize)) > 0) {
            len += cnt;
            int res = 0;
            char* p = buf;
            while(cnt > 0) {
                res = w.write(p, cnt);
                if (res < 0) return -1;
                p += res;
                cnt -= res;
            }
        }   
        if (cnt < 0) return -1;
        return len;
    }

    ssize_t sendTo(Writeable& w, size_t len) {
        char buf[kFileBufferSize] = {0};
        int want = std::min(len, kFileBufferSize);
        int all = 0;
        int cnt = 0;

        while(want > 0 && (cnt = read(buf, want)) > 0) {
            all += cnt;
            want = std::min(len - all, kFileBufferSize);
            
            int res = 0;
            char* p = buf;
            while(cnt > 0) {
                res = w.write(p, cnt);
                if (res < 0) return -1; 
                p += res;
                cnt -= res;
            }
        }
        if (cnt < 0) return -1;
        return len;
    }

    ssize_t write(const char* buf, size_t buf_len) {
        return append(buf, buf_len);
    }

    ssize_t append(const char* buf, size_t buf_len) {
        openWrite();
        int fd = fWriteFd;
        if (fd < 0) return -1;
        int rs = ::write(fd, buf, buf_len);
        return rs;
    }

    int unlink() {
        return ::unlink(fFullName.data());
    }

    virtual ~File() {
        this->close();
    }

    int closeRead() {
        return ::close(fReadFd);
    }

    int closeWrite() {
        return ::close(fWriteFd);
    }

    int closeLock() {
        return ::close(fLockFd);
    }

private:
    void close() {
        closeRead();
        closeWrite();
        closeLock();
    }

private:
    std::string fFullName;
    int fReadFd;
    int fWriteFd;
    int fLockFd;
    bool fLocked;
    ssize_t fLabelLength = -1;

};

} //namespace nattan

#endif
