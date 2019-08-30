/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_CORE_MEMORYDATA_H
#define NATTAN_CORE_MEMORYDATA_H

#include "core/Data.h"

namespace nattan {

class MemoryData: public Data {

public:
    virtual size_t length() = 0;
    virtual ssize_t read(char* buf, size_t buf_len) = 0;
    virtual ssize_t write(const char* buf, size_t buf_len) = 0;
    virtual char* data() = 0;
    virtual void clear() = 0;

    char readChar() {
        char c;
        read(&c, 1);
        return c;
    }

    int readInt() {
        int v;
        char* buf = (char*)(&v);
        int len = sizeof(v); 
        read(buf, len);
        return v;
    }

    int readLong() {
        long g;
        char* buf = (char*)(&g);
        int len = sizeof(g);
        read(buf, len);
        return g;
    }

    int writeChar(const char c) {
        return write(&c, 1);
    }

    int writeInt(const int x) {
        char* buf = (char*)(&x);
        size_t len = sizeof(x);
        return write(buf, len);
    }

    int writeLong(const long x) {
        char* buf = (char*)(&x);
        size_t len = sizeof(x);
        return write(buf, len);
    }

    char peekChar() {
        char c = readChar();
        writeChar(c);
        return c;
    }

    int peekInt() {
        int v = readInt();
        writeInt(v);
        return v;
    }

    long peekLong() {
        long g = readLong();
        writeLong(g);
        return g;
    }

};

} // namespace nattan

#endif

