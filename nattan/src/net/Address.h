/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_NET_ADDRESS_H
#define NATTAN_NET_ADDRESS_H

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace nattan {

struct Address {
    int domain = AF_INET;
    int type = SOCK_STREAM;
    int family = AF_INET;
    std::string address;
    int port;

    Address(){}
    Address(const std::string& addr, const unsigned int p): address(addr), port(p) {}
    Address(sockaddr_in& sock_addr, const int fam = AF_INET) {
        char addr[256] = {0};
        family = fam;
        socklen_t addr_len = sizeof(addr);
        ::inet_ntop(family,(const void*)(&(sock_addr.sin_addr)), addr, addr_len);
        address = addr;
        port = ntohs(sock_addr.sin_port);
    }

    int toSockAddr(sockaddr_in& sock_addr) const {
        sock_addr.sin_family = family;
        sock_addr.sin_port = htons(port);
        if (address == ADDRESS_INET_ANY) {
            sock_addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
        } else {
            sock_addr.sin_addr.s_addr = ::inet_addr(address.data());
        }
        return 0;
    }

    static std::string ADDRESS_INET_ANY;
};

std::string Address::ADDRESS_INET_ANY = "0.0.0.0";
} // namespace nattan

#endif
