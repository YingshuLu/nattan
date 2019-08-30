#ifndef NATTAN_PROTO_DNS_DNS_H
#define NATTAN_PROTO_DNS_DNS_H

#include <poll.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <vector>
#include <ares.h>
#include <time.h>
#include <stdio.h>
#include "task/Task.h"
#include "log/Log.h"

extern "C" {
#include "deps/libcask/sys_helper.h"
}

namespace nattan {

static void __dns_callback(void* arg, int status, int timeouts, unsigned char* abuf, int alen);

class DNS {
friend void __dns_callback(void* arg, int status, int timeouts, unsigned char* abuf, int alen);

public:
    DNS() {}

    bool resolve(const char* hostname) {
        if (!init()) return false;
        return query(hostname);
    }

    const char* getPreferIP() const {
        if (fResults.empty()) return NULL;
        return fResults[0].first.data();
    }

    const std::vector<std::pair<std::string, int>>& getResolvedResult() const {
        return fResults;
    }

    ~DNS() {
        destroy();
    }

private:
    bool init() {
        if (bInited) destroy();

        int status = ares_init_options(&fChannel, &fOptions, 0);
        if (status != ARES_SUCCESS) {
            return false;
        }
        bInited = true;
        return true;
    }

    bool query(const char* hostname) {
        ares_query(fChannel, hostname, ns_c_in, ns_t_a, __dns_callback, this);

        while(true) {
            fd_set read_fds, write_fds;
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);
            int nfds = ares_fds(fChannel, &read_fds, &write_fds);
            if (nfds == 0) break;

            // select start
            {
                std::vector<struct pollfd> pollfds;
                for (int fd = 1; fd < nfds; fd++) {
                    if(FD_ISSET(fd, &read_fds) || FD_ISSET(fd, &write_fds)) {
                        struct pollfd pfd;
                        pfd.fd = fd;
                        pfd.events = 0;
                        pfd.events |= (FD_ISSET(fd, &read_fds)? EPOLLIN : 0);
                        pfd.events |= (FD_ISSET(fd, &write_fds)? EPOLLOUT : 0);
                        pollfds.push_back(pfd);
                    }
                }
                int n = co_poll(pollfds.data(), pollfds.size(), 1000);

                //timeout
                if (n == 0) {
                    break;
                }

                DBG_LOG("dns poll %d socket", n);
                FD_ZERO(&read_fds);
                FD_ZERO(&write_fds);
                for (int i = 0; i < n; i++) {
                    if (pollfds[i].revents & POLLIN) {
                        FD_SET(pollfds[i].fd, &read_fds);
                    }

                    if (pollfds[i].revents & POLLOUT) {
                        FD_SET(pollfds[i].fd, &write_fds);
                    }           
                }
            }
            // select end
            ares_process(fChannel, &read_fds, &write_fds);
        }   
        return !(fResults.empty());
    }

    void destroy() {
        if (!bInited) return;
        ares_destroy(fChannel);
        ares_library_cleanup();
    }

public:
    bool bInited = false;
    time_t fTimeout;
    ares_channel fChannel;
    struct ares_options fOptions;
    std::vector<std::pair<std::string, int>> fResults;
};

static void __dns_callback(void* arg, int status, int timeouts, unsigned char* abuf, int alen) {
    if (status != ARES_SUCCESS) {
        return;
    }

    DNS* pdns = (DNS*) arg;
    if (pdns)
    {
        struct ares_addrttl addrttls[10];
        int naddrttls = 10;
        int s = ares_parse_a_reply(abuf, alen, NULL, addrttls, &naddrttls);
        if (s != ARES_SUCCESS) return;
        char ip[INET6_ADDRSTRLEN];
        for (int i = 0; i < naddrttls; i++) {
            inet_ntop(AF_INET, &(addrttls[i].ipaddr), ip, INET6_ADDRSTRLEN);
            pdns->fResults.push_back(std::make_pair(ip, addrttls[i].ttl));
        }
    }
}

} //namespace nattan
#endif
