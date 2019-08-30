/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_BASE_CLOSEABLE_H
#define NATTAN_BASE_CLOSEABLE_H

namespace nattan {

class Closeable {
public:
    virtual int close() = 0;

protected:
    Closeable() {};
    virtual ~Closeable() {}
};

} // namespace nattan
#endif
