/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_PROTO_HTTP_HTTPPARSER_H
#define NATTAN_PROTO_HTTP_HTTPPARSER_H

#include <string>
#include "proto/Parser.h"
#include "proto/http/HttpHeader.h"
#include "proto/http/HttpBody.h"
#include "deps/http-parser/http_parser.h"

namespace nattan {

int __http_on_message_begin(http_parser*);
int __http_on_message_complete(http_parser*);
int __http_on_headers_complete(http_parser*);
int __http_on_url(http_parser*, const char*, size_t);
int __http_on_status(http_parser*, const char*, size_t);
int __http_on_header_field(http_parser*, const char*, size_t);
int __http_on_header_value(http_parser*, const char*, size_t);
int __http_on_body(http_parser*, const char*, size_t);

class HttpParser: public Parser{
// callbacks
friend int __http_on_message_begin(http_parser*);
friend int __http_on_message_complete(http_parser*);
friend int __http_on_headers_complete(http_parser*);
friend int __http_on_url(http_parser*, const char*, size_t);
friend int __http_on_status(http_parser*, const char*, size_t);
friend int __http_on_header_field(http_parser*, const char*, size_t);
friend int __http_on_header_value(http_parser*, const char*, size_t);
friend int __http_on_body(http_parser*, const char*, size_t);

public:
    HttpParser(http_parser_type type, const size_t limit = 6 * 1024): 
        header(type, limit) {}

    int parse(const char* buf, size_t buf_len) {
        if(!bInited) init();
        return http_parser_execute(&fParser, &fSettings, buf, buf_len);
    }

    ssize_t sendTo(Writeable& w) {
        ssize_t res = header.sendTo(w);
        if (res < 0) return res;
        ssize_t ret = body.sendTo(w);
        if (ret < 0) return ret;
        return res + ret;
    }

    const char* getHost() {
        return header.getHeader("Host");    
    }

    void setHost(const char* hostname) {
        if (!hostname) return;
        header.setHeader("Host", hostname);     
    }

    bool keepalive() {
        return bKeepalive;
    }

    bool headersCompleted() {
        return bHeadersCompleted;
    }

    bool eof() {
        return bEndOfMessage;
    }

    void reset() {
        bEndOfMessage = false;
        bHeadersCompleted = false;
        bHeadRequest = false;
        bKeepalive = false;
        bInited = false;

        header.reset();
        body.reset();
        fHeaderKey.clear();
    }

    HttpHeader header;
    HttpBody body;

protected:
    void setHeadRequest() {
        bHeadRequest = true;
    }

private:
    void init() {
        http_parser_init(&fParser, header.type);
        http_parser_settings_init(&fSettings);
        fParser.data = static_cast<void*>(this);
        
        fSettings.on_message_begin    = __http_on_message_begin;
        fSettings.on_message_complete = __http_on_message_complete;
        fSettings.on_headers_complete = __http_on_headers_complete;

        fSettings.on_url              = __http_on_url;
        fSettings.on_status           = __http_on_status;
        fSettings.on_header_field     = __http_on_header_field;
        fSettings.on_header_value     = __http_on_header_value;
        fSettings.on_body             = __http_on_body;

        //init success
        bHeadersCompleted = false;
        bKeepalive = false;
        bHeadersCompleted = false;
        bEndOfMessage = false;
        bInited = true;
    }

private:
    http_parser fParser;
    http_parser_settings fSettings;
    bool bEndOfMessage = false;
    bool bHeadRequest = false;
    bool bKeepalive = false;
    bool bHeadersCompleted = false;
    bool bInited = false;

private:
    std::string fHeaderKey;

};

class HttpRequest: public HttpParser {

public:
    HttpRequest(): HttpParser(HTTP_REQUEST) {}

};

class HttpResponse: public HttpParser {

public:
    HttpResponse(): HttpParser(HTTP_RESPONSE) {}

};

extern int __http_on_message_begin(http_parser* parser) {
    return 0;
}

extern int __http_on_message_complete(http_parser* parser) {
    HttpParser* p = static_cast<HttpParser*>(parser->data);
    p->bEndOfMessage = true;
    return 0;
}

extern int __http_on_headers_complete(http_parser* parser) {
    HttpParser* p = static_cast<HttpParser*>(parser->data);
    if(p->header.type == HTTP_REQUEST) p->header.method = parser->method;
    else {
        p->header.status_code = parser->status_code;
    }

    p->bKeepalive = (http_should_keep_alive(parser));
    p->bHeadersCompleted = true;

    if (!(parser->http_major > 0 && parser->http_minor > 0)) p->header.version = HttpHeader::HTTP_10;
    if (p->header.type == HTTP_RESPONSE && p->bHeadRequest) return 1;  
    return 0;
}

extern int __http_on_url(http_parser* parser, const char* at, size_t len) {
    HttpParser* p = static_cast<HttpParser*>(parser->data);
    std::string s(at, len);
    p->header.uri = s;
    return 0;
}

extern int __http_on_status(http_parser* parser, const char* at, size_t len) {
    HttpParser* p = static_cast<HttpParser*>(parser->data);
    p->header.status_desc = std::string(at, len);
    return 0;
}

extern int __http_on_header_field(http_parser* parser, const char* at, size_t len) {
    HttpParser* p = static_cast<HttpParser*>(parser->data);
    p->fHeaderKey = std::string(at, len);
    return 0;
}

extern int __http_on_header_value(http_parser* parser, const char* at, size_t len) {
    HttpParser* p = static_cast<HttpParser*>(parser->data);
    p->header.setHeader(p->fHeaderKey, std::string(at, len));
    return 0;
}

extern int __http_on_body(http_parser* parser, const char* at, size_t len) {
    HttpParser* p = static_cast<HttpParser*>(parser->data);
    p->body.write(at, len);
    return 0;
}

} // namespace nattan
#endif
