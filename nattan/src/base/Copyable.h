/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_BASE_COPYABLE_H
#define NATTAN_BASE_COPYABLE_H

namespace nattan {

class Copyable {
protected:
    Copyable() {};
    virtual ~Copyable() {};
};

} // namespace nattan

#endif
