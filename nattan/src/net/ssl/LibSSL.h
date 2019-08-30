/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_NET_SSL_LIBSSL_H
#define NATTAN_NET_SSL_LIBSSL_H

#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <vector>
#include <memory>
#include "thread/Thread.h"
#include "thread/ThreadMutex.h"
#include "net/ssl/Ciphers.h"

namespace nattan {

namespace _libssl_ {
 
static std::vector<std::unique_ptr<ThreadMutex>> g_MutexArray;

static unsigned long __thread_id() {
	return (unsigned long)Thread::gettid();
}

static void __lock_callback(int mode, int type, const char *file, int line) {
	if(mode & CRYPTO_LOCK)
		g_MutexArray[type]->lock();
  	else
		g_MutexArray[type]->unlock();
}

static bool __g_openssl_inited = false;
static bool OpenSSLThreadSafeInit() {
	if (__g_openssl_inited) return true;
	for(int i = 0; i < CRYPTO_num_locks(); i++) {
		std::unique_ptr<ThreadMutex> mutex_ptr(new ThreadMutex());
		g_MutexArray.push_back(std::move(mutex_ptr));
	}
	CRYPTO_set_id_callback(__thread_id);
	CRYPTO_set_locking_callback(__lock_callback);
	return true;
}

static SSL_CTX* g_client_ssl_ctx_list[5] = {NULL};
static SSL_CTX* g_server_ssl_ctx_list[5] = {NULL};

#define NATTAN_SSLV3 0x0300
#define NATTAN_TLS10 0x0301
#define NATTAN_TLS11 0x0302
#define NATTAN_TLS12 0x0303
#define NATTAN_SSL_ANY   0x0004

static SSL_CTX* NewClientSSLCtx(short version) {
    SSL_CTX *ctx = nullptr;
    switch(version) {
        case NATTAN_SSLV3:
            ctx = ::SSL_CTX_new(SSLv3_client_method());
            break;
        case NATTAN_TLS10:
            ctx = ::SSL_CTX_new(TLSv1_client_method());
            break;
        case NATTAN_TLS11:
            ctx = ::SSL_CTX_new(TLSv1_1_client_method());
            break;
        case NATTAN_TLS12:
            ctx = ::SSL_CTX_new(TLSv1_2_client_method());
            break;
        default:
            ctx = ::SSL_CTX_new(SSLv23_client_method());
    }
    ::SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);
    return ctx;
}

static SSL_CTX* NewServerSSLCtx(short version) {
    SSL_CTX *ctx = nullptr;
    switch(version) {
        case NATTAN_SSLV3:
            ctx = ::SSL_CTX_new(SSLv3_server_method());
            break;
        case NATTAN_TLS10:
            ctx = ::SSL_CTX_new(TLSv1_server_method());
            break;
        case NATTAN_TLS11:
            ctx = ::SSL_CTX_new(TLSv1_1_server_method());
            break;
        case NATTAN_TLS12:
            ctx = ::SSL_CTX_new(TLSv1_2_server_method());
            break;
        default:
            ctx = ::SSL_CTX_new(SSLv23_server_method());
    }
    ::SSL_CTX_set_cipher_list(ctx, "ALL");
    ::SSL_CTX_set_ecdh_auto(ctx, 1);
    return ctx;
}

static void FreeSSLCtx(SSL_CTX *ctx) {
    ::SSL_CTX_free(ctx);
}

static SSL_CTX* GetClientSSLCtx(short version) {
        int idx = version & 0x000F;
        SSL_CTX* ctx = g_client_ssl_ctx_list[idx];
        if (!ctx) ctx = NewClientSSLCtx(version);
        g_client_ssl_ctx_list[idx] = ctx;
        return ctx;
}

static SSL_CTX* GetServerSSLCtx(short version) {
        int idx = version & 0x000F;
        SSL_CTX* ctx = g_server_ssl_ctx_list[idx];
        if (!ctx) ctx = NewServerSSLCtx(version);
        g_server_ssl_ctx_list[idx] = ctx;
        return ctx;       
}

static bool LibsslInit() {
     OpenSSLThreadSafeInit();
    ::SSL_load_error_strings();
    ::ERR_load_crypto_strings();
    ::OpenSSL_add_ssl_algorithms();
    return true;
}

static bool bLibsslInited = LibsslInit();

const char* GetCipherByHexCode(const int hexcode) {
    auto it = g_ssl_ciphers.find(hexcode);
    if(it != g_ssl_ciphers.end()) {
        const char* std_name = it->second;
        auto iter = g_standard_openssl_ciphers_map.find(std_name);
        if (iter != g_standard_openssl_ciphers_map.end()) {
            return iter->second;
        }
    }
    return NULL;
}

}// namespace _libssl_

} // namespace nattan

#endif
