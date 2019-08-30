/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_PROTO_HTTP_HTTPBODY_H
#define NATTAN_PROTO_HTTP_HTTPBODY_H

#include <string>
#include <memory>
#include "core/TempFile.h"
#include "net/Socket.h"

namespace nattan {

enum HTTPBODY_MODE {
    HTTPBODY_TEMP,
    HTTPBODY_DISK
};

class HttpBody: public Data {

public:

    HttpBody(const size_t memory_limit = 1024 * 1024): 
        fData(memory_limit), pFile(nullptr), fMode(HTTPBODY_TEMP){} 

    size_t length() {
        if (fMode == HTTPBODY_DISK) return pFile->length();
        return fData.length();
    }

    ssize_t read(char* buf, size_t buf_len) {
        if (fMode == HTTPBODY_DISK) return pFile->read(buf, buf_len);
        return fData.read(buf, buf_len);
    }

    ssize_t write(const char* buf, size_t buf_len) {
        if (fMode == HTTPBODY_DISK) return pFile->write(buf, buf_len);
        return fData.write(buf, buf_len);
    }

    using Data::write;

    ssize_t sendTo(Writeable& w) {
        if (length() == 0) return 0;
        ssize_t ret = 0;
        if (fMode == HTTPBODY_DISK) ret = pFile->sendTo(w);
        else ret = fData.sendTo(w);
        if (ret < 0) return ret;
        if(w.write("\r\n", 2) < 0) return -1;
        return ret;
    }

    ssize_t dumpAsFile(File& file) {
        size_t len = length();
        char buf[65536] = {0};
        
        int ret = 0;
        int cnt = 0;
        int left = 0;
        char *p = buf;
        while((ret = read(buf, 65536)) > 0) {
            p = buf;
            left = ret;
            while(left > 0 && (cnt = file.write(p, left)) > 0) {
                p += cnt;
                left -= cnt;
                cnt = 0;
            }
            if (cnt < 0) return -1;
        }       
        if (ret < 0) return -1;
        return len;
    }

    void useFile(std::shared_ptr<File>& file) {
        pFile = file;
        fMode = HTTPBODY_DISK;
    }

    void useFile(const std::string& filename) {
        pFile = std::shared_ptr<File>(new File(filename));
        fMode = HTTPBODY_DISK;
    }

    std::shared_ptr<File> getFile() {
        if (fMode == HTTPBODY_DISK) return pFile;
        return nullptr;
    }
    
    void reset() {
        pFile = nullptr;
        fData.clear();
        fMode = HTTPBODY_TEMP;
    }

private:
    TempFile fData;
    std::shared_ptr<File> pFile;
    HTTPBODY_MODE fMode;
};

} // namespace nattan

#endif
