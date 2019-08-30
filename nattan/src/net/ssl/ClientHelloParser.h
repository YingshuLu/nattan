/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_NET_SSL_CLIENTHELLOPARSER_H
#define NATTAN_NET_SSL_CLIENTHELLOPARSER_H

#include <arpa/inet.h>
#include <vector>
#include <string>
#include "log/Log.h"
#include "net/ssl/LibSSL.h"

namespace nattan {

#pragma pack(1)
struct Recoder {
    char type;
    short version;
    short length;
};

struct HandshakeClientHello {
    char type;
    char length[3];
    short version;
    char random[32];
    char session_length;
};
#pragma pack()

class ClientHelloParser {

public:
     bool eof() const {
        return bCompleted;
     }

     int parse(char *data, size_t length) {
        DBG_LOG("parse data length: %d", length);
        // need more 
        if (length < sizeof(Recoder)) return 1;

        char * cur = data;
        Recoder* recoder = (Recoder*) cur;
        recoder->version = ntohs(recoder->version);
        recoder->length = ntohs(recoder->length);

        cur += sizeof(Recoder);
        DBG_LOG("Recoder type: %d", recoder->type);
        DBG_LOG("Recoder version: %d", recoder->version);
        DBG_LOG("Recoder length: %d", recoder->length);
 
        // invalid
        if (recoder->type != CONTENT_TYPE_HANDSHAKE) return -1;

        // need more 
        size_t need = recoder->length + sizeof(Recoder);
        if (length < need) return 1;

        HandshakeClientHello* clientHello = (HandshakeClientHello*)(cur);
        clientHello->version = ntohs(clientHello->version);
        fVersion - clientHello->version;

        DBG_LOG("handshake type: %d", clientHello->type);
        DBG_LOG("handshake version: %d", clientHello->version);
        DBG_LOG("client hello session length: %d", clientHello->session_length);

        cur = cur + sizeof(HandshakeClientHello) + clientHello->session_length;
        short ciphers_length = *((short *)(cur));
        cur += 2;
        ciphers_length = ntohs(ciphers_length);
        DBG_LOG("ciphers length: %d", ciphers_length);

        short* ciphers = (short *)(cur);
        int n = ciphers_length / 2;
        for (int i = 0; i < n; i++) {
            DBG_LOG("client Hello cipher : %04x, name: %s", ntohs(ciphers[i]), _libssl_::GetCipherByHexCode(ntohs(ciphers[i])));
            fCiphers.push_back(ntohs(ciphers[i]));
        }

        cur += ciphers_length;
        char compression_length = cur[0];
        DBG_LOG("compression length: %d", compression_length);
        cur ++;
        cur += compression_length;
        short extension_length = ntohs(*((short*)(cur)));
        DBG_LOG("extension length: %d", extension_length);
        cur += 2; 

        char * end = data + length;
        while(cur < end) {
            short type = ntohs(*((short*) cur));
            DBG_LOG(">> extension type: %u", type);
            cur += 2;
            short length = ntohs(*((short*)cur));
            DBG_LOG(">> extension length: %u", length);
            cur += 2;

            if (type == short(0)) {
                short list_length = ntohs(*((short*)cur));
                DBG_LOG(">> name list length: %d", list_length);
                cur += 2;
                char type = cur[0];
                DBG_LOG(">> name type: %d", type);
                cur ++;
                short name_length = ntohs(*((short*)cur));
                DBG_LOG(">> name length: %d", name_length);
                cur += 2;
                
                fServer = std::string(cur, cur + name_length);
                DBG_LOG(">> server name: %s", fServer.data());
                cur += name_length;
                break;
            }
            else {
                cur += length;
            }
        }

        bCompleted = true;
        return 0;
     }

     short version() const {
        return fVersion;
     }

     const char* SNI() {
        return fServer.data();
     }

     const std::vector<int>& ciphers() {
        return fCiphers;
     }

     void reset() {
        bCompleted = false;
        fVersion = -1;
        fCiphers.clear();
     }

private:
     short fVersion = -1;
     std::vector<int> fCiphers;
     std::string fServer;
     bool bCompleted = false;
     static const char CONTENT_TYPE_HANDSHAKE      = 22;
     static const char HANDSHAKE_TYPE_CLIENT_HELLO =  1;
};

} // namespace nattan
#endif
