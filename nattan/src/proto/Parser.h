
#ifndef NATTAN_PROTO_PARSER_H
#define NATTAN_PROTO_PARSER_H

namespace nattan {

class Parser {
public:
    virtual int parse(const char* data, size_t len) = 0;
};

} // namespace nattan

#endif
