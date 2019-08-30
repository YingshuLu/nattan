/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_BASE_READABLE_H
#define NATTAN_BASE_READABLE_H

#include <stddef.h>
#include <unistd.h>

namespace nattan {

class Readable {
public:
    virtual ssize_t read(char* buf, size_t len) = 0;

protected:
    Readable() {};
    virtual ~Readable() {};
};

} //namespace nattan
#endif
