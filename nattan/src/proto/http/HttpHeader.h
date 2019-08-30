/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_PROTO_HTTP_HTTPHEADER_H
#define NATTAN_PROTO_HTTP_HTTPHEADER_H

#include <string>
#include <map>
#include "core/Buffer.h"
#include "deps/http-parser/http_parser.h"

namespace nattan {

const char* HTTP_VERSION_STR[] = {
    "HTTP/0.9",
    "HTTP/1.0",
    "HTTP/1.1"
};

class HttpHeader: public Data {

public:
enum HTTP_VERSION {
    HTTP_09,
    HTTP_10,
    HTTP_11
};

public:
    HttpHeader(http_parser_type t, const size_t limit_size = 8 * 1024):
        version(HTTP_11),
        type(t),
        kHeaderLimitSize(limit_size),
        buffer(limit_size) {}

    size_t length() {
        return buffer.length();
    }

    ssize_t read(char* buf, size_t buf_len) {
        return buffer.read(buf, buf_len);
    }

    ssize_t write(const char* buf, size_t buf_len) {
        return buffer.write(buf, buf_len);
    }

    ssize_t recvFrom(Readable& r) {
        char* buf = buffer.end();
        size_t buf_len = buffer.right();
        ssize_t res = r.read(buf, buf_len);
        if (res > 0) buffer.fill(res);
        return res;
    }

    ssize_t sendTo(Writeable& w) {
        dumpToBuffer();
        return buffer.sendTo(w);
    }

    char* data() { return buffer.data(); }

    const char* getHeader(const std::string& key) {
        auto it = fHeaders.find(key);
        if(it != fHeaders.end()) {
            return it->second.data();
        }
        return NULL;
    }

    void setHeader(const std::string& key, const std::string& value) {
        if (key.empty() || value.empty()) return;
        fHeaders[key] = value;  
    }

    bool dumpToBuffer() {
        buffer.clear();
        if (HTTP_REQUEST == type) {
            const char* m = http_method_str((http_method)method);
            buffer.write(m, strlen(m));
            buffer.writeChar(' ');
            buffer.write(uri.data(), uri.length());
            buffer.writeChar(' ');
            const char* v = HTTP_VERSION_STR[version];
            buffer.write(v, strlen(v));
            buffer.writeChar('\r');
            buffer.writeChar('\n');
        } else {
            const char* v = HTTP_VERSION_STR[version];
            buffer.write(v, strlen(v));
            buffer.writeChar(' ');
            std::string s = std::to_string(status_code);
            buffer.write(s);
            buffer.writeChar(' ');
            if (status_desc.empty()) status_desc = http_status_str((http_status)status_code);
            buffer.write(status_desc);
            buffer.writeChar('\r');
            buffer.writeChar('\n');
        }

        for(auto it = fHeaders.begin(); it != fHeaders.end(); it ++) {  
            buffer.write(it->first);
            buffer.writeChar(':');
            buffer.writeChar(' ');
            buffer.write(it->second);
            buffer.writeChar('\r');
            buffer.writeChar('\n');     
        }
        buffer.writeChar('\r');
        buffer.writeChar('\n');
        if (buffer.length() > kHeaderLimitSize) return false;
        return true;
    }

    bool exceedLimit() {
        return buffer.length() > kHeaderLimitSize;
    }

    void reset() {
        buffer.clear();
        fHeaders.clear();
    }

public:
    int version;
    int method;
    int status_code;
    std::string uri;
    std::string status_desc;

public:
    Buffer buffer;
    http_parser_type type;

private:
    const size_t kHeaderLimitSize;
    std::map<std::string, std::string> fHeaders;
};

} // namespace nattan;

#endif 
