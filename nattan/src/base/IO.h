/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_BASE_IO_H
#define NATTAN_BASE_IO_H

#include <time.h>
#include "base/Readable.h"
#include "base/Writeable.h"
#include "base/Closeable.h"

namespace nattan {

class IO: public Readable, public Writeable, public Closeable {
protected:
    IO() {}
    virtual ~IO() {}
};

} // namespace nattan
#endif
