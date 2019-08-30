/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_NET_SOCKETHELPER_H
#define NATTAN_NET_SOCKETHELPER_H

#include <poll.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <algorithm>
#include "base/Final.h"
#include "net/Socket.h"
#include "net/Address.h"
#include "net/Unhook.h"
#include "core/Buffer.h"

namespace nattan {

class SocketHelper: public Final {

public:

    static SOCKET TcpConnect(const Address& addr) {
        SOCKET sockfd = ::socket(addr.domain, addr.type, 0);

        do {
            Socket sock(sockfd);
            if (!sock.valid()) {
                sockfd = INVALID_SOCKET;
                break;
            }
            sockaddr_in sock_addr;  
            addr.toSockAddr(sock_addr);
            int n = ::connect(sockfd, (sockaddr*) (&sock_addr), sizeof(sockaddr_in));
            if (n < 0) {
                sock.close();
                sockfd = INVALID_SOCKET;
                break;
            }
            setupSockAddress(sockfd, addr);
            TcpSetKeepalive(sock, true);
            TcpSetNoDelay(sock, true);
            sock.setCloseOnExec();
        } while(0);

        return sockfd;
    }

    static SOCKET TcpConnectTill(const Address& addr, time_t ms) {
        SOCKET sockfd = ::socket(addr.domain, addr.type, 0);

        Unhook unhook;
        do {
            Socket sock(sockfd);
            if (!sock.valid()) {
                sockfd = INVALID_SOCKET;
                break;
            }
            sockaddr_in sock_addr;  
            addr.toSockAddr(sock_addr);
            int ret = ::connect(sockfd, (sockaddr*) (&sock_addr), sizeof(sockaddr_in));
            if(ret == -1) {
                if(errno == EALREADY || errno == EINPROGRESS) {
                    struct pollfd p;
                    p.fd = sockfd;
                    p.events = EPOLLOUT | EPOLLRDHUP | EPOLLERR;
                    int n = co_poll(&p, 1, ms);
                    if (n == 1){
                       if (p.revents & EPOLLOUT) {
                           // success
                           if(!get_connect_error(sockfd)) {
                                setupSockAddress(sockfd, addr);
                                TcpSetKeepalive(sock, true);
                                TcpSetNoDelay(sock, true);
                                sock.setCloseOnExec();
                                break;
                           }
                       } 
                    }
                }
                sock.close();
                sockfd = INVALID_SOCKET;
                break;
            }
            else {
                // success
                setupSockAddress(sockfd, addr);
                TcpSetKeepalive(sock, true);
                TcpSetNoDelay(sock, true);
                sock.setCloseOnExec();
            }
        } while(0);

        return sockfd;
    }

    static SOCKET TcpListen(const Address& addr) {
        bool failed = false;
        SOCKET sockfd = INVALID_SOCKET;
        Socket sock;
        do {
            sockfd = ::socket(addr.domain, addr.type, 0);
            sock.attach(sockfd);
            if (!sock.valid()) {
                failed = true;
                break;
            }

            sock.setReuseAddress();
            sock.setReusePort();
            sockaddr_in sock_addr;
            addr.toSockAddr(sock_addr);
            int n = ::bind(sockfd, (sockaddr*)(&sock_addr), sizeof(sockaddr_in));
            if (n < 0) {
                failed = true;
                break;
            }

            int backlog = 10;
            n = ::listen(sockfd, backlog); 
            if (n < 0) {
                failed = true;
                break;
            }
            
            // setup sockfd address
            setupSockAddress(sockfd, addr);
        } while(0);

        if (failed) {
            sock.close();
        }
        return sockfd;
    }   

    static SOCKET TcpAccept(const SOCKET sockfd) {
        Socket clientSock;
        SOCKET client_sockfd = INVALID_SOCKET;

        do {
            sockaddr_in client_addr;
            socklen_t addr_len = sizeof(sockaddr_in);
            client_sockfd = ::accept(sockfd, (sockaddr*)(&client_addr), &addr_len);
            clientSock.attach(client_sockfd);
            if (!clientSock.valid()) {
                client_sockfd = INVALID_SOCKET;
                clientSock.close();
                break;
            }
            Address addr(client_addr);
            setupSockAddress(client_sockfd, addr);
        } while(0);

        return client_sockfd;
    }

    static bool TcpSetNoDelay(const SOCKET sockfd, const bool on) {
        if (!Socket::valid(sockfd)) return false;
        int yes = on? 1 : 0;
        return ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(int)) == 0;
    }

    static bool TcpSetCork(const SOCKET sockfd, const bool on) {
        if (!Socket::valid(sockfd)) return false;
        int yes = on? 1 : 0;
        return ::setsockopt(sockfd, IPPROTO_TCP, TCP_CORK, &yes, sizeof(int)) == 0;
    }

    static bool TcpSetKeepalive(const SOCKET sockfd, const bool on) {
        if (!Socket::valid(sockfd)) return false;
        int yes = on? 1 : 0;
        return ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) == 0;
    }

    // to do
    static int tunnel(const SOCKET csock, const SOCKET ssock, time_t timeout_ms) {
        if (!Socket::valid(csock) || !Socket::valid(ssock)) return -1;

        bool use_ssl = (nullptr != Socket::GetSSLHandler(csock)) && (nullptr != Socket::GetSSLHandler(ssock));
        struct pollfd pfds[2];
        pfds[0].fd = csock;
        pfds[0].events = EPOLLIN | EPOLLRDHUP | EPOLLERR;

        pfds[1].fd = csock;
        pfds[1].events = EPOLLIN | EPOLLRDHUP | EPOLLERR;
        
        int len = 1024 * 10;
        int from = -1;
        int to = -1;
        while(true) {
            int n = co_poll(pfds, 2, timeout_ms);
            //timeout
            if (n == 0) {
                return n;
            }
            else if (n < 0) {
                return n;
            }
            from = pfds[0].fd;
            to = from == csock? ssock : csock;

            if (use_ssl) {
                if (SpliceSSLData(Socket::GetSSLHandler(from), Socket::GetSSLHandler(to), len, timeout_ms) < 0) return -1;
            }
            else {
                if (SpliceSocketData(from, to, len, timeout_ms ) < 0) return -1;
            }
        }
        return 0;
    }

    static int WaitForSocketReadable(const SOCKET sockfd, const time_t timeout_ms) {
        return WaitForSocketEvent(sockfd, EPOLLIN, timeout_ms);
    }

    static int WaitForSocketWritable(const SOCKET sockfd, const time_t timeout_ms) {
        return WaitForSocketEvent(sockfd, EPOLLOUT, timeout_ms);
    }

private:
    //to do
    static int SpliceSocketData(const SOCKET from, const SOCKET to, int max_len, time_t timeout_ms) {
        int pipefds[2];
        if (pipe2(pipefds, O_NONBLOCK) < 0) return -1;
        int fd_in  = pipefds[0];
        int fd_out = pipefds[1];
        size_t len = 32 * 1024;
        if (max_len < 0) max_len = len;
        else max_len = std::min(max_len, (int)len);

        int recv_len = 0;
        do {
            recv_len = ::splice(from, NULL, fd_in, NULL, max_len, SPLICE_F_MOVE | SPLICE_F_NONBLOCK | SPLICE_F_MORE);
            if (recv_len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    int res = WaitForSocketReadable(from, timeout_ms);
                    if (res <= 0) return -1;
                    continue;
                }
                return -1;
            }
        } while(recv_len <= 0);
        
        int data_len = recv_len;
        int send_len = 0;
        while(recv_len > 0) {
            send_len = ::splice(fd_out, NULL, to, NULL, recv_len, SPLICE_F_MOVE | SPLICE_F_NONBLOCK | SPLICE_F_MORE);
            if (send_len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    int res = WaitForSocketWritable(to, timeout_ms);
                    if (res <= 0) return -1;
                    continue;
                }
                return -1;
            }
            recv_len -= send_len;
        }
        
        return data_len;
    }

    static int SpliceSSLData(SSLHandler* from, SSLHandler* to, int max, time_t timeout_ms) {
        if (!from || !to) return -1;
        int from_fd = from->getSocket();
        int to_fd = to->getSocket();

        Unhook unhook;
        int len = std::max(max, 10 * 1024);
        Buffer buf(len);
        int recvn = 0;
        do {
            recvn = from->read(buf.end(), buf.right());
            if (recvn <= 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    int res = -1;
                    if (from->getErrno() == SSL_ERROR_WANT_WRITE) {
                        res = WaitForSocketWritable(from_fd, timeout_ms);
                    }
                    else if (from->getErrno() == SSL_ERROR_WANT_READ) {
                        res = WaitForSocketReadable(from_fd, timeout_ms);
                    }
                    // error
                    if (res <= 0) return -1;
                    continue;
                }
                //error
                return -1;
            }
        } while(recv < 0); 
        buf.fill(recvn);

        int data_tunnel = recvn;
        int sendn = -1; 
        while(buf.length() > 0) {
            sendn = to->write(buf.data(), buf.length()); 
            if (sendn <= 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    int res = -1;
                    if (to->getErrno() == SSL_ERROR_WANT_WRITE) {
                        res = WaitForSocketWritable(from_fd, timeout_ms);
                    }
                    else if (to->getErrno() == SSL_ERROR_WANT_READ) {
                        res = WaitForSocketReadable(from_fd, timeout_ms);
                    }
                    // error
                    if (res <= 0) return -1;
                    continue;
                }
                //error
                return -1;
            }
            buf.shrink(sendn);    
        }
        return data_tunnel;
    }

    static int WaitForSocketEvent(const SOCKET sockfd, const int event, const time_t timeout_ms) {
        struct pollfd pfd;
        pfd.fd = sockfd;
        pfd.events = event | EPOLLRDHUP | EPOLLERR;
        return co_poll(&pfd, 1, timeout_ms);
    }


private:
    static void setupSockAddress(const SOCKET sockfd, const Address& addr) {
        Address* pAddr = new Address(addr); 
        g_sock_address[sockfd] = pAddr;
    }

    static void cleanupSockAddress(const SOCKET sockfd) {
        Address* pAddr = g_sock_address[sockfd];
        g_sock_address[sockfd] = nullptr;
        delete pAddr;
    }
};

} //nattan

#endif
