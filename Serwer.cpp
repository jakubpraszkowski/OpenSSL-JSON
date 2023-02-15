#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>
#include <winsock2.h> 
#include <Windows.h> 
#include <Ws2tcpip.h> 

#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

int create_socket(int port)
{
    WSADATA wsaData;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "error WSAStartup" << WSAGetLastError() << std::endl;
        return NULL;
    }

    int s;
    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof addr);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0) {
        perror("Unable to create socket");
        WSACleanup();
        return NULL;
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        closesocket(s);
        WSACleanup();
        return NULL;
    }

    if (listen(s, 1) < 0) {
        perror("Unable to listen");
        closesocket(s);
        WSACleanup();
        return NULL;
    }

    return s;
}

SSL_CTX* create_context()
{
    const SSL_METHOD* method;
    SSL_CTX* ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    return ctx;
}

bool configure_context(SSL_CTX* ctx)
{
    if (SSL_CTX_use_certificate_file(ctx, "my.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "my.pass.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (SSL_CTX_load_verify_locations(ctx, "my.crt", nullptr) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
    return true;
}

int main(int argc, char** argv)
{
    int sock;
    SSL_CTX* ctx;

    ctx = create_context();
    if (!ctx) {
        exit(EXIT_FAILURE);
    }

    bool result;
    result = configure_context(ctx);
    if (!result) {
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    sock = create_socket(8080);
    if (!sock) {
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    while (1) {
        struct sockaddr_in addr;
        int len = sizeof(addr);
        SSL* ssl;

        char buf[1024];
        string wyniki;
        int bytes;

        int client = accept(sock, (struct sockaddr*)&addr, &len);
        if (client >= 0)
        {
            cout << "Connection accepted" << endl;
        }
        if (client < 0) {
            perror("Unable to accept");
            continue;
        }

        ssl = SSL_new(ctx);
        if (!ssl) {
            perror("Error when creating SSL");
            continue;
        }

        int res;
        res = SSL_set_fd(ssl, client);
        if (!res) {
            perror("Error when connecting SSL object with a file descriptor");
            continue;
        }

        int rozmiar = 0;
        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        }
        else {
            do
            {
                bytes = SSL_read(ssl, buf, 1023);
                if (bytes > 0)
                {
                    buf[bytes] = 0;
                    printf("Data: \"%s\"\n", buf);
                    wyniki.append(buf, bytes);
                }
                else if (bytes == 0)
                {
                    printf("No data to receive\n");
                    break;
                }
                else
                {
                    perror("Unable to read");
                    break;
                }
            } while (bytes > 0);

        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        closesocket(client);
    }

    SSL_CTX_free(ctx);
    closesocket(sock);
    WSACleanup();
    return 0;
}
