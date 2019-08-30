/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_CORE_TEMPFILE_H
#define NATTAN_CORE_TEMPFILE_H

#include <algorithm>
#include "core/Buffer.h"
#include "core/File.h"
#include "core/Data.h"
#include "util/UUID.h"
#include "base/Writeable.h"

namespace nattan {

static const std::string kTempFileDir = "/tmp/";
static const size_t kTempFileMemoryLimit = 1024 * 1024;

enum TEMPFILE_MODE {
    TEMPFILE_BUFFER,
    TEMPFILE_FILE
};

class TempFile: public Data{

public:
    TempFile(const size_t limit = kTempFileMemoryLimit):
        fFile(kTempFileDir + std::to_string(UUID::get())), kMemoryLimit(limit), fMode(TEMPFILE_BUFFER){
            fFile.openRead();
            fFile.openWrite();
    }   

    TempFile(const std::string tmp_file, const size_t limit = kTempFileMemoryLimit): 
        fFile(kTempFileDir + tmp_file), kMemoryLimit(limit), fMode(TEMPFILE_BUFFER) {
            fFile.openRead();
            fFile.openWrite();
    }

    void setDiskMode() {
        dumpToFile();       
    }

    void setMemoryMode() {
        fMode = TEMPFILE_BUFFER;
    }

    size_t length() {
        switch(fMode) {
            case TEMPFILE_BUFFER: {
                return fBuf.length();
            }
            case TEMPFILE_FILE: {
                return fFile.length();
            }
        }
        return 0;   
    }

    ssize_t read(char* buf, size_t buf_len) {
        switch(fMode) {
            case TEMPFILE_BUFFER: {
                return fBuf.read(buf, buf_len);
            }

            case TEMPFILE_FILE: {
                return fFile.read(buf, buf_len);
            }
        }
        return -1;
    }

    ssize_t write(const char* buf, size_t buf_len) {
        while(true) {
            switch (fMode) {
                case TEMPFILE_BUFFER: {
                    if (fBuf.length() + buf_len > kMemoryLimit) {
                        dumpToFile();
                        continue;
                    }
                    else {
                        return fBuf.write(buf, buf_len);
                    }
                }
                case TEMPFILE_FILE: {
                    return fFile.write(buf, buf_len);
                }
            }
        }
        return -1;
    }

    ssize_t sendTo(Writeable& w) {
        switch(fMode) {
            case TEMPFILE_BUFFER: {
                return fBuf.sendTo(w);
            }
            case TEMPFILE_FILE: {
                return fFile.sendTo(w);
            }
        }
        return -1;
    }

    void clear() {
        fFile.unlink();
        fBuf.clear();
        fMode = TEMPFILE_BUFFER;
    }

    ~TempFile() {
        fFile.unlink();
    }

private:
    ssize_t dumpToFile() {
        char buf[4096] = {0};
        int n = -1;
        int len = fBuf.length();
        while(fBuf.length() > 0) {
            n = fBuf.read(buf, 4096);   
            if (n < 0) return -1;
            fFile.write(buf, n);
        }
        fMode = TEMPFILE_FILE;
        return len;
    }

private:
    TEMPFILE_MODE fMode;
    Buffer fBuf;
    File fFile;
    const size_t kMemoryLimit;
};

} //namespace nattan

#endif 
