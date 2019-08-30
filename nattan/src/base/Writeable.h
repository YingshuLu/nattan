/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_BASE_WRITEABLE_H
#define NATTAN_BASE_WRITEABLE_H

#include <stddef.h>
#include <unistd.h>
#include <string>
#include <vector>

namespace nattan {

class Writeable {
public:

    virtual ssize_t write(const char* buf, size_t len) = 0;

    ssize_t write(const std::string& buf) {
        return write(buf.data(), buf.length());
    }

    size_t write(const std::vector<char>& buf) {
        return write(buf.data(), buf.size());
    }

protected:
    Writeable() {};
    virtual ~Writeable() {};
};

} //namespace nattan
#endif



