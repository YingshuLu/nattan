/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_NET_SOCKET_H
#define NATTAN_NET_SOCKET_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <time.h>
#include "base/Uncopyable.h"
#include "base/IO.h"
#include "net/Address.h"
#include "net/ssl/SSLHandler.h"

extern "C" {
#include "deps/libcask/sys_helper.h"
#include "deps/libcask/inner_fd.h"
}

namespace nattan {

typedef int SOCKET;
const SOCKET INVALID_SOCKET = -1;
const size_t SOCKET_MAX_NUMBER = 65536; 
static Address* g_sock_address[SOCKET_MAX_NUMBER] = {nullptr};
static SSLHandler* g_sock_ssl_handler[SOCKET_MAX_NUMBER] = {nullptr};

class Socket: public IO, public Uncopyable {
friend class SocketHelper;

public:
    Socket() {}
    explicit Socket(const SOCKET fd): fSockFd(fd) {}

    Socket& operator=(const SOCKET fd) {
        fSockFd = fd;
        return *this;
    }

    operator SOCKET() {
        return fSockFd;
    }

    virtual ~Socket(){}

    SOCKET getSocket() const {
        return fSockFd;
    }
    
    int detach() {
        SOCKET sockfd = fSockFd;
        fSockFd = INVALID_SOCKET;
        return sockfd;
    }

    void attach(const SOCKET sock) {
        fSockFd = sock;
    }

    bool setReuseAddress() {
        if (!valid()) return false;
        int yes = 1;
        return ::setsockopt(fSockFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == 0;
    }
        
    bool setReusePort() {
        if (!valid()) return false;
        int yes = 1;
        return ::setsockopt(fSockFd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(int)) == 0;
    }

    bool setCloseOnExec() {
        if (!valid()) return false;
        int flags = ::fcntl(fSockFd, F_GETFD);
        flags |= FD_CLOEXEC;
        ::fcntl(fSockFd, F_SETFD, flags);
        return true;    
    }

    static bool valid(SOCKET sockfd) {
        if (sockfd <= INVALID_SOCKET) return false;
        int error = 0;
        socklen_t len = sizeof(error);
        int rs = ::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
        if (rs != 0) return false;
        if (error != 0) return false;
        return true;
    }

    bool valid() {
        return valid(fSockFd);
    }

    const Address* getAddress() {
        if (!valid()) return nullptr;
        return g_sock_address[fSockFd];
    }

    void setTimeout(time_t tm) {
        set_inner_fd_timeout(fSockFd, tm);  
        fTimeout = tm;
    }

    time_t getTimeout() {
        return fTimeout;
    }

    virtual ssize_t read(char *buf, size_t buf_len) {
        if (fSockFd < 0) return -1;
        if (getSSLHandler()) {
            return getSSLHandler()->read(buf, buf_len);
        }
        return ::read(fSockFd, buf, buf_len);
    }

    virtual ssize_t write(const char* buf, size_t buf_len) {
        if (fSockFd < 0) return -1;
        if (getSSLHandler()) {
            return getSSLHandler()->write(buf, buf_len);
        }
        return ::write(fSockFd, buf, buf_len);
    }

    int readTill(char *buf, size_t buf_len, time_t ms) {
        return co_read_till(fSockFd, buf, buf_len, ms);
    }

    int writeTill(const char *buf, size_t buf_len, time_t ms) {
        return co_write_till(fSockFd, buf, buf_len, ms);
    }
    
    bool setSSLHandler(SSLHandler* ssl) {
        if(fSockFd < 0) return false; 
        g_sock_ssl_handler[fSockFd] = ssl;
        return true;
    }

    SSLHandler* getSSLHandler() {
        return GetSSLHandler(fSockFd);
    }

    static SSLHandler* GetSSLHandler(const SOCKET sockfd) {
       if (sockfd < 0) return nullptr;
       return g_sock_ssl_handler[sockfd];
    }

    virtual int close() {
        if (fSockFd < 0) return -1;
        if (g_sock_address[fSockFd] != nullptr) {
            delete g_sock_address[fSockFd];
            g_sock_address[fSockFd] = nullptr;
        }

        if (getSSLHandler()) {
            SSLHandler* ssl = getSSLHandler();
            g_sock_ssl_handler[fSockFd] = nullptr;
            ssl->close();
            delete ssl;
        }

        int rs = ::close(fSockFd);
        fSockFd = INVALID_SOCKET;
        return rs;
    }

protected:
    SOCKET fSockFd = INVALID_SOCKET;    
    time_t fTimeout = 0;
};

} // namespace nattan

#endif
