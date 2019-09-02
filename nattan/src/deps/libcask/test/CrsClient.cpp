#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <arpa/inet.h>
extern "C" {
#include "co_define.h"
}
#include "TmCrsMessage.h"

#define MAXBUF 65536

const char *JWT_TOKEN = "eyJhbGciOiJSUzI1NiIsImtpZCI6IktsNS1GOUVwMldyWVY1OGF2OEQyQjFETTB6ZkhhNE91bGhkbGswbnk4eXciLCJ0eXAiOiJKV1QifQ.eyJhdWQiOiJJV1NIIiwiZXhwIjoxNTY5ODM2NzU3LCJpYXQiOjE1NjIwNjA3NTcsImlzcyI6IklXU0giLCJzdWIiOiJodHRwczovL2Nycy50cmVuZG1pY3JvLmNvbSIsInRtVG9rZW5JZCI6InB1dC10b2tlbi1pZC1oZXJlIn0.AFfVFb7QAHWB8F7l7StzhNWIwI155DBMnGvYez8xLqT9rsVjGWx71uJyT3UuolKpT6XZOf2j3R3PdQklsYbtG4EzU5jqmEjaat-egInl_BMthkAm0cxlM-A1JAs_8_29hCHEa2UptducDQLFL4-eKqCD8PYvTdfhbsPom9VP0B_ITqdLH2vN7OSiuWuJnSU5h4PZK0_BN8By4DBLYMMf0l3mfG_NtUYtGMvlfOMBJcv8_2I1j_0I4HTA_scmSxw-vAWQDTj0SM4WxiOTpxHqdrCEsumWDKA3XnK8NZxrTjk_U7FJPMep7i-sIAUcfKT4rKUeYnMTyE8eow98STHpRw"; 

int resolveHost(const char* host, char *buf) {
    INF_LOG("DNS query host: %s", host);
    struct hostent *net = gethostbyname(host);
    DBG_LOG("gethostbyname complete");
    if(net) {
        const char *h = net->h_addr_list[0];
        int i = 0;
        if (h) {
			char *ip = inet_ntoa(*((struct in_addr*)(net->h_addr_list)[i]));
            INF_LOG("host: %s, resolve at address: %s", host, ip);
			strcpy(buf, ip);
			return 0;
        }
    }
    else {
        INF_LOG("failed to resolve host: %s", host);
    }
	return -1;
}

void ShowCerts(SSL * ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        INF_LOG("数字证书信息:");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        INF_LOG("证书: %s", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        INF_LOG("颁发者: %s", line);
        free(line);
        X509_free(cert);
    } else
        INF_LOG("无证书信息！");
}

int ssl_client()
{
    //co_disable_hook();
    int sockfd, len;
    struct sockaddr_in dest;
    char buffer[MAXBUF + 1];
    SSL_CTX *ctx;
    SSL *ssl;

	const char* host = "testing.crs.trendmicro.com";
    ctx = SSL_CTX_new(TLSv1_2_client_method());
    //ctx = SSL_CTX_new(SSLv3_client_method());
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }



	CrsSingleRequest sq;
	sq.setQueryServiceId();
	sq.setQueryServiceName();
	sq.setQueryValue("www.cnblogs.com");
	sq.setQueryType(CRS_INFORMATION::CRS_QUERY_BY_FQDN);

	CrsRequest req;
	req.append(sq);

	sq.setQueryValue("www.baidu.com");
	req.append(sq);
	std::string creq = req.toString();

	char ipaddr[256] = {0};
	if (resolveHost(host, ipaddr) != 0) return -1;

    /* 创建一个socket 用于tcp 通信*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket");
        exit(errno);
    }
    INF_LOG("socket created");

    /* 初始化服务器端（对方）的地址和端口信息*/
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(443);
    if (inet_aton(ipaddr, (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
        perror("failed to init ipaddr");
        exit(errno);
    }
    INF_LOG("address created");

    /* 连接服务器*/
    if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
        perror("Connect ");
        exit(errno);
    }
    INF_LOG("server connected");

    /* 基于ctx 产生一个新的SSL */
    ssl = SSL_new(ctx);
	SSL_set_tlsext_host_name(ssl, host); 
    SSL_set_fd(ssl, sockfd);

    /* 建立SSL 连接*/
    if (SSL_connect(ssl) == -1)
        ERR_print_errors_fp(stderr);
    else {
        INF_LOG("Connected with %s encryption", SSL_get_cipher(ssl));
        ShowCerts(ssl);
    }
    /* 接收对方发过来的消息，最多接收MAXBUF 个字节*/
    bzero(buffer, MAXBUF + 1);

    const char *post_fmt = "POST /crs/v1/graphql HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nAuthorization: Bearer %s\r\nContent-Length: %ld\r\n\r\n%s\r\n";

	char post[65536] = {0};
	
	int post_len = snprintf(post, 65536, post_fmt, host, JWT_TOKEN, creq.length(), creq.data()); 

	INF_LOG("send %d bytes request: \n%s", post_len, post);

    /* 发消息给服务器*/
    len = SSL_write(ssl, post, post_len);
    if (len < 0)
        INF_LOG
        ("消息'%s'发送失败！错误代码是%d，错误信息是'%s'",
         buffer, errno, strerror(errno));
    else
        INF_LOG("消息'%s'发送成功，共发送了%d 个字节！",
               buffer, len);


    /* 接收服务器来的消息*/
    len = SSL_read(ssl, buffer, MAXBUF);
    if (len > 0)
        INF_LOG("接收消息成功:'%s'，共%d 个字节的数据",
               buffer, len);
    else {
        INF_LOG
        ("消息接收失败！错误代码是%d，错误信息是'%s'",
         errno, strerror(errno));
        goto finish;
    }
    bzero(buffer, MAXBUF + 1);
    strcpy(buffer, "from client->server");
finish:
    /* 关闭连接*/
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);

    return 0;
}

int mykey;

void client(void* ip, void *op) {
    /*
    //co_enable_hook();
    int *value = (int *)malloc(sizeof(int));
    *value = 124;
    co_spec_set(mykey, value);
    value = co_spec_get(mykey);
    DBG_LOG("set value: %d", *value);
    */

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sock_addr;
    socklen_t addr_len = sizeof(sock_addr);
    bzero(&sock_addr, addr_len);

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(8848);
    sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    int ret = connect(sockfd, (struct sockaddr *)&sock_addr, addr_len);
    if(ret) {
        dperror(ret);
        return;
    }

    char* request = "GET / HTTP/1.1\rHost: www.tigerso.com\r\n\r\n";
    size_t len = strlen(request);
    while(len) {
        ret = write(sockfd, request, strlen(request));
        if(ret < 0) {
            dperror(ret);
            close(sockfd);
            return;
        }
        len -= ret;
    }

    INF_LOG("send request:%s", request);

    char* response = (char *) malloc(65536);
    bzero(response, 65536);
    ret = read(sockfd, response, 65536);
    if(ret < 0) {
        dperror(ret);
        return;
    }

    INF_LOG("recv response:%s", response);
    free(response);
    close(sockfd);
    return;
}

void myssl(void *ip, void *op) {
    //co_disable_hook();
    ssl_client();
}

int main() {
    
    /* SSL 库初始化，参看ssl-server.c 代码*/
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    int i = 0;
    for(; i < 2; i++) {
        co_create(myssl, NULL, NULL);
    }

    schedule();
    return 0;
}
