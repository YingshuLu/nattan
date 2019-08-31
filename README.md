# nattan
c++ network library on coroutine, sync non-blocking API for high performace

Coroutine of nattn is a wrapper of libcask for C++,  nattan provides analog synchronize API.
Any IO block would lead to coroutines schudle in thread, and provide thread-level coroutine sync mechanism,
like coroutine mutex, spin lock, blocking array (channel in golang), timer, etc.

To use nattan,  the project should only include the header files, instead of using any binary static / dynamic libary.
nattan has implemented several common protocols based on non-blocking sync API, i.e. http(s), DNS, redis-client, zookeepr-cli.

## DNS resolve

```
#include "proto/dns/DNS.h"

class DNSTask : public Task {

public:
  void run() {
      ...
      DNS dns;
      const char* domain = "www.google.com";
      dns.resolve(domain);
  }

};
```
