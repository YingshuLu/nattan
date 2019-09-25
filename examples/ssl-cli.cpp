
#include <string.h>
#include "log/Logger.h"
#include "net/SocketHelper.h"
#include "task/Task.h"
#include "net/ssl/SSLHelper.h"
#include "proto/http/HttpParser.h"
#include "proto/dns/DNS.h"

using namespace nattan;

Logger logger("./https.log");


class HttpsClientTask: public Task {

public:

    HttpsClientTask(const char* hostname) {
        host = hostname;    
    }

    void run() {
        const char* hostname = host.data();
        DNS dns;
        if (!dns.resolve(hostname)) {
            logger.info("DNS resolve %s failed", hostname);
            return;
        } 

        Address addr(dns.getPreferIP(), 443);

        SOCKET sockfd = SocketHelper::TcpConnect(addr);

        if (!Socket::valid(sockfd)) {
            logger.info("connect failed");
            return;
        }

        Socket sock(sockfd);

        SSLHandler* ssl = SSLHelper::CreateClientSSLHandler(sockfd);

        if (!ssl) {
            logger.info("failed to create ssl");
            return;
        }

        ssl->setServerName(hostname);

        if (SSLHelper::Connect(ssl) != 0) {
            logger.info("failed to handshake");
        }
        
        sock.setSSLHandler(ssl);

        Buffer buf(4096);
        request.header.method = HTTP_GET;
        request.setHost(hostname);
        request.header.setHeader("Agent", "nattan/1.0");
        request.header.uri = "/";

        int n = request.sendTo(sock);
        if (n <= 0) {
            sock.close();
            return;
        }
        logger.info("send request, %d bytes", n);
        while(!response.eof()) {
            n = sock.read(buf.end(), buf.right());
            if(n < 0) {
                logger.info("failed to recv response");
                return;
            }
            buf.fill(n);
            n = response.parse(buf.data(), buf.length());
            if (n < 0) {
                logger.info("failed to parse response");
                return;
            }
            buf.shrink(n);
        }

        response.header.dumpToBuffer();
        logger.info("response header: %s", response.header.buffer.data());
        sock.close();
        

    }
private:
    HttpRequest request;
    HttpResponse response;
    std::string host;

};


int main(int argc, char* argv[] ) {
    if (argc < 2) return 0;
    const char* hostname = argv[1];
    Task *ptask = new HttpsClientTask(hostname);

    ptask->start();
    logger.info("ptask start");
    Task::sched();
    return 0;
}

