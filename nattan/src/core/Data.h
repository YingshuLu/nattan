/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_CORE_DATA_H
#define NATTAN_CORE_DATA_H

#include "base/Copyable.h"
#include "base/Readable.h"
#include "base/Writeable.h"
#include <string>

namespace nattan {

class Data {

public:
    virtual size_t length() = 0;
    virtual ssize_t read(char* buf, size_t buf_len) = 0;
    virtual ssize_t write(const char* buf, size_t buf_len) = 0;

    ssize_t write(const std::string& buf) {
        return write(buf.data(), buf.length());
    }

protected:
    Data(){}
    virtual ~Data(){}

};

} //namespace nattan

#endif

