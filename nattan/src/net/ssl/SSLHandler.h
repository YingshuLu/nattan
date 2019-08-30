/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/
#ifndef NATTAN_NET_SSL_SSLHANDLER_H
#define NATTAN_NET_SSL_SSLHANDLER_H

extern "C" {
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/md5.h>
}
#include "net/ssl/LibSSL.h"
#include "base/Uncopyable.h"

namespace nattan {

class SSLHandler : public Uncopyable {
public:
     enum SSLType {
        SSL_CLIENT,
        SSL_SERVER
     };

public:
    SSLHandler() {}

    int open(const SSLType type, const int sockfd, const short version = NATTAN_SSL_ANY) {
        if (sockfd <= 0) return -1;
        SSL_CTX *ctx = nullptr;
        switch(type) {
            case SSLType::SSL_CLIENT:
                ctx =  _libssl_::GetClientSSLCtx(version);
                break;
            case SSLType::SSL_SERVER:
                ctx = _libssl_::GetServerSSLCtx(version);
                break;
            default:
                return -1;
        }

       fssl = ::SSL_new(ctx);
       if (!fssl) return -1;
       int res = ::SSL_set_fd(fssl, sockfd);
       if (res != 1) return -1;
       return 0;
    }

    int getSocket() {
        return ::SSL_get_fd(fssl);
    }

    int setServerName(const char* serverName) {
        if (!serverName) return -1;
        if (::SSL_set_tlsext_host_name(fssl, serverName) != 1) return -1;
        return 0;
    }

    bool setCipherListInteger(const std::vector<int>& ciphers) {
         std::string cipher_strs;
         const char* s = NULL;
         for(auto it = ciphers.begin(); it != ciphers.end(); it++) {
            s = _libssl_::GetCipherByHexCode(*it);
            if (s) {
                if (cipher_strs.empty()) {
                    cipher_strs.append(s);
                } else {
                    cipher_strs.append(":");
                    cipher_strs.append(s);
                }
            }
         }
        return ::SSL_set_cipher_list(fssl, cipher_strs.data()) == 1;
    }

    bool setCipherListString(const std::vector<std::string>& ciphers) {
        std::string cipher_strs;
        for(auto it = ciphers.begin(); it != ciphers.end(); it++) {
            if(cipher_strs.empty()) {
                cipher_strs.append(*it);
            } else {
                cipher_strs.append(":");
                cipher_strs.append(*it);
            }
        }
        return ::SSL_set_cipher_list(fssl, cipher_strs.data()) == 1;
    }

    const char *getServerName() {
        return ::SSL_get_servername(fssl, SSL_get_servername_type(fssl));
    }

    int read(char* buf, size_t buf_len) {
        return ::SSL_read(fssl, buf, buf_len);
    }

    int write(const char* buf, size_t buf_len) {
        return ::SSL_write(fssl, buf, buf_len);
    }

    int setupCertPkey(X509* cert, EVP_PKEY* pkey) {
        int res = ::SSL_use_certificate(fssl, cert);
        if (1 != res) {
            return -1;
        }

        res = ::SSL_use_PrivateKey(fssl, pkey);
        if (1 != res) {
            return -1;
        }

        return 0;
    }

    int getErrno() {
        if (!fssl) return 0;
        return ::SSL_get_error(fssl, 0);
    }

    int close() {
        if (fssl) {
            ::SSL_shutdown(fssl);
        }
        freeResources();
        return 0;
    }

    SSL* getSSL() {
        return fssl;
    }

    ~SSLHandler() {
        if (!fssl) close();
    }

private:
    void freeResources() {
        if (fssl) {
            ::SSL_free(fssl);
            fssl = nullptr;
        }

        if (fcert) {
            ::X509_free(fcert);
            fcert = nullptr;
        }

        if (fpkey) {
            ::EVP_PKEY_free(fpkey);
            fpkey = nullptr;
        }
    }

private:
    SSL* fssl = nullptr;
    X509* fcert = nullptr;
    EVP_PKEY* fpkey = nullptr;
};

} // namespace nattan
#endif
