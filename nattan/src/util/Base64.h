/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_UTIL_BASE64_H
#define NATTAN_UTIL_BASE64_H

#include <string>
#include <vector>
#include "util/base64/base64.h"
#include "base/Final.h"

namespace nattan {

class Base64 : public Final{

public:
    static std::string encode(const std::string& content) {
        if (content.empty()) return std::string();
        int encode_len = Base64encode_len(content.length());
        std::vector<char> buf(encode_len + 1, 0);

        Base64encode(buf.data(), content.data(), content.length());
        std::string result = buf.data();
        return result;  

    }

    static std::string decode(const std::string& content) {
        std::string result;
        if (content.empty()) return result;
        int decode_len = Base64decode_len(content.data());
        std::vector<char> buf(decode_len + 1, 0);

        Base64decode(buf.data(), content.data());
        result = buf.data();
        return result;
    }

};

} // namespace nattan

#endif
