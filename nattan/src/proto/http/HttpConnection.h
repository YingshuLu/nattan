#ifndef NATTAN_PROTO_HTTP_HTTPCONNECTION_H
#define NATTAN_PROTO_HTTP_HTTPCONNECTION_H

#include "net/Socket.h"
#include "core/Buffer.h"
#include "proto/http/HttpParser.h"

namespace nattan {

class HttpConnection {

public:

    HttpConnection() {}
    HttpConnection(SOCKET sockfd): fSock(sockfd) {}
    HttpConnection(Socket& sock): fSock(sock.detach()) {}

    void attach(SOCKET sockfd) {
        fSock.attach(sockfd);
    }

    bool alive() {
        return fSock.valid();
    }

    bool send(HttpParser& parser) {
        parser.sendTo(fSock);
        return true;
    }

    bool recv(HttpParser& parser) {
        fBuf.clear();
        while(!parser.eof()) {
            ssize_t n = fSock.read(fBuf.end(), fBuf.right());
            if (n <= 0) return false;
            fBuf.fill(n);
            int p = parser.parse(fBuf.data(), fBuf.length());
            if (p <= 0) return false;
            fBuf.shrink(p);
        }
        return true;
    }

    bool close() {
        return fSock.close() == 0;
    }

protected:
    Socket fSock;

private:
    Buffer fBuf;
};

} //namespace nattan

#endif
