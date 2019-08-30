/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_BASE_UNCOPYABLE_H
#define NATTAN_BASE_UNCOPYABLE_H

namespace nattan {

class Uncopyable {
protected:
    Uncopyable(){}
    ~Uncopyable(){}

private:
    Uncopyable(const Uncopyable& uc) {};
    Uncopyable& operator=(const Uncopyable& uc) { return *this; };
};

} // namespace nattan
#endif
