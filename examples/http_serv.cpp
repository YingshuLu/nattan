#include "task/Task.h"
#include "task/TaskThread.h"
#include "net/Socket.h"
#include "core/Buffer.h"
#include "thread/Runnable.h"
#include "proto/http/HttpParser.h"
#include "net/SocketHelper.h"
#include "log/Logger.h"
#include "log/Log.h"
#include "core/File.h"
#include "proto/dns/DNS.h"
#include <string>
#include <algorithm>
#include "net/ssl/ClientHelloParser.h"
#include "net/Poller.h"

using namespace nattan;

Logger logger("./http.log");

class TcpServer {

class ClientRunner: public Runnable {

public:
    ClientRunner(const SOCKET sockfd): fSock(sockfd) {}

    bool send(Socket& sock, HttpParser& parser) {
        parser.sendTo(sock);
        return true;
    }

    bool recv(Socket& sock, HttpParser& parser) {
        Buffer buf;
        while(!parser.eof()) {
            int n = sock.read(buf.end(), buf.right());
            logger.info("recv:\n%s\n", buf.end());
            if (n <= 0) return false;
            buf.fill(n);
            int p = parser.parse(buf.data(), buf.length());
            if (p <= 0) return false;
            buf.shrink(p);
        }
        return true;
    }

    void run() 
    {
        logger.info("processing socket [%d]", fSock.getSocket());    
        HttpRequest request;
        HttpResponse response;

        while(true) {
            request.reset();
            response.reset();
            if (!recv(fSock, request)) {
                break;
            }

            response.header.status_code = 200;
            bool keepalive = request.keepalive();
            response.header.setHeader("Connection", keepalive? "close" : "Keep-Alive");
            response.setHost("nattan.co");
            std::string content = "hello nattan!";
            response.body.write(content.data(), content.size()); 
            response.header.setHeader("Content-length", std::to_string(response.body.length()));

            if (!send(fSock, response)) {
                return;
            }

            if (!keepalive) {
                break;
            }
        }
        fSock.close();
    }

private:
    Socket fSock;
    Socket clientSock;
};

class DispatchRunner: public Runnable {
public:
    DispatchRunner(const Address& addr) {
        fAddr = addr;
    }

    void run() {
        SOCKET serverFd = SocketHelper::TcpListen(fAddr);
        fSock.attach(serverFd);
        bool running = true;
        while(running) {
            SOCKET clientFd = SocketHelper::TcpAccept(fSock.getSocket());
            logger.info("dispatch accept socket [%d]", clientFd);
            TaskThread* thread = TaskThread::currentTaskThread();
            std::shared_ptr<Runnable> runner(new ClientRunner(clientFd));
            thread->submit(runner);
        }

    }
private:
    Socket fSock;
    Address fAddr;
};

public:
    TcpServer(const Address& addr) {
        fAddr = addr;
    }

    bool service() {
        {
            // start work task threads
            for (int i = 0; i < 1; i++) {
                std::shared_ptr<TaskThread> threadPtr(new TaskThread(1000 * 60));
                threadPtr->start();
                fTaskThreads.push_back(threadPtr);
            }
    
            // push accept runner
            for(auto it = fTaskThreads.begin(); it != fTaskThreads.end(); it++)
            {
                std::shared_ptr<Runnable> runner(new DispatchRunner(fAddr));
                while(!((*it)->submit(runner)));
            }
            logger.info("%d task threads start", fTaskThreads.size());
        }

        for(auto it = fTaskThreads.begin(); it != fTaskThreads.end(); it++) {
            (*it)->join();
        }
    }

private:
    Address fAddr;
    std::vector<std::shared_ptr<TaskThread>> fTaskThreads;

};

int main() {
    DevelopLog::switchOn();
    DevelopLog::setDebugFile("./http.log");

    Address addr("0.0.0.0", 8001);
    TcpServer server(addr);
    server.service();
}
