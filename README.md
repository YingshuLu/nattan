# nattan
c++ network library on coroutine, sync non-blocking API for high performace

Coroutine of nattn is a wrapper of libcask for C++,  nattan provides analog synchronize API.
Any IO block would lead to coroutines schudle in thread, and provide thread-level coroutine sync mechanism,
like coroutine mutex, spin lock, blocking array (channel in golang), timer, etc.

To use nattan,  the project should only include the header files, instead of using any binary static / dynamic libary.
nattan has implemented several common protocols based on non-blocking sync API, i.e. http(s), DNS, redis-client, zookeepr-cli.

## DNS resolve

```
#include "nattan/proto/dns/DNS.h"
#include "natan/task/Task"

class DNSTask : public Task {

public:
  void run() {
      ...
      DNS dns;
      const char* domain = "www.google.com";
      const char* ip = nullptr;
      if(dns.resolve(domain)) {
        ip = dns.getPreferIP();
      }
  }

};
```

## Receive / Send http message

```
#include "core/Buffer.h"
#include "proto/http/HttpParser.h"

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
```

HttpRequest and HttpResponse extends from HttpParser

## Coroutine pool per thread

```
#include "task/TaskThread.h"
#include "thread/Runnable.h"

// corutine would be killed if no task runned after 60 seconds
TaskThread pool(60 * 1000);
pool.start();
...

class Runner: public Runnable {
 public:
    void run() {
      // your code
    }
};

std::shared_ptr<Runner> runnerPtr = std::make_shared<Runner>();
pool.submit(runnerPtr);

```
TaskThread implements corutine-level pool, corutines would be reused to run Runnable and
may be killed with a specific idle time.

## Coroutine schedule
please refer to my another libcask: https://github.com/YingshuLu/libcask

## Inter coroutine sync
Like thread sync, the ways of corutines's listed as:
```
    sync/SpinLock.h
    sync/TaskMutex.h
    sync/TaskCond.h
    sync/Chan.h
```

## CPU consumed function
About CPU-consumed function called in coroutine, it can not be scheduled for  
other coroutines, nattan provide ##await## interface for such issue:  

```
class CPURunner: public runnable {
    ...
};

class MyTask: public Task {

public:
    void run() {
        ...
        CPURunner cpu_runner;
        Task.await(cpu_runner);
        ...
    }

};

``` 
await implemented by thread-pool on background, would block caller coroutine
until the Runnable exit. 

## Third-party Hook
below hooked third party can be used safely, nattan would convert its API to Async non-blocking IO.  

redis client    :   https://github.com/redis/hiredis  
zookeeper client:   https://github.com/apache/zookeeper/tree/master/zookeeper-client/zookeeper-client-c  
c-ares          :   https://github.com/c-ares/c-ares  
openssl         :   https://github.com/openssl/openssl  
mysql client    :   https://dev.mysql.com/downloads/connector/c/  

Note: please use their sync API with nattan.
DNS in nattan is using c-ares, and nattan has implemented self's SSL wrapper net/ssl/SSLHandler.h, and
it can also implement SSL inspection with forge certificate.
