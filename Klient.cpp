#pragma comment(lib, "libcurl_imp.lib")
#pragma comment(lib, "jsoncpp.lib")#include <curl/curl.h>

#include <json/json.h>

#include <iostream>

#include <winsock2.h>

#include <Windows.h>

#include <Ws2tcpip.h>

#include <openssl/sha.h>

#include <openssl/ssl.h>

#include <openssl/err.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

static size_t Writeback(void * contents, size_t size, size_t nmemb, void * userp) {
  size_t rozmiar;
  rozmiar = size * nmemb;
  ((std::string * ) userp) -> append((char * ) contents, rozmiar);
  return rozmiar;
}

int create_socket(int port) {
  WSADATA wsaData;
  int iResult = WSAStartup(MAKEWORD(2, 2), & wsaData);
    
  if (iResult != 0) {
    std::cout << "error WSAStartup" << WSAGetLastError() << std::endl;
    return 0;
  }
    
  int s;
  struct sockaddr_in addr;
  memset( & addr, '\0', sizeof addr);
  addr.sin_family = AF_INET;
  int result;
  result = inet_pton(AF_INET, "127.0.0.1", & (addr.sin_addr));
    
  if (!result) {
    perror("Unable to make a conversion");
    return 0;
  }
    
  addr.sin_port = htons(port);
  s = socket(AF_INET, SOCK_STREAM, 0);
    
  if (s < 0) {
    perror("Unable to create socket");
    WSACleanup();
    return 0;
  }
    
  if (connect(s, (struct sockaddr * ) & addr, sizeof(addr)) < 0) {
    perror("Unable to connect");
    closesocket(s);
    WSACleanup();
  }
  return s;
}

SSL_CTX * create_context() {
  const SSL_METHOD * method;
  SSL_CTX * ctx;
  method = TLS_client_method();
  ctx = SSL_CTX_new(method);
    
  if (!ctx) {
    perror("Unable to create SSL context");
    ERR_print_errors_fp(stderr);
    return NULL;
  }
  return ctx;
}

void our_cleanup(SSL * ssl, CURL * curl, SSL_CTX * ctx, int sock) {
  if (ssl) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }
  if (curl) curl_easy_cleanup(curl);
  if (ctx) SSL_CTX_free(ctx);
  if (sock > 0) closesocket(sock);
  WSACleanup();
}

int main(void) {
    char buf[1024];
    string odebrane;
    int bytes;
    int sock = -1;
    SSL_CTX * ctx = nullptr;
    SSL * ssl = nullptr;
    CURL * curl = nullptr;
    CURLcode res;
    std::string str;
    Json::Reader reader;
    Json::Value js;
    char * znak;
    ctx = create_context();
    
    if (!ctx) {
      perror("Error when creating context");
      exit(EXIT_FAILURE);
    }
    
    sock = create_socket(8080);
    if (!sock) {
      perror("Error creating socket");
      our_cleanup(ssl, curl, ctx, sock);
      exit(EXIT_FAILURE);
    }
    
    ssl = SSL_new(ctx);
    if (!ssl) {
      perror("Error when creating SSL");
      our_cleanup(ssl, curl, ctx, sock);
      exit(EXIT_FAILURE);
    }
    
    int resull;
    resull = SSL_set_fd(ssl, sock);
    if (!resull) {
      perror("Error when connecting SSL object with a file descriptor");
      our_cleanup(ssl, curl, ctx, sock);
      exit(EXIT_FAILURE);
    }
    
    curl = curl_easy_init();
    if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, "https://api.open-
        meteo.com / v1 / forecast ? latitude = 50.33 & longitude = 19.13 & current_weather = true ");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1 L); curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Writeback); curl_easy_setopt(curl, CURLOPT_WRITEDATA, & str); res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
          fprintf(stderr, "Curle no okie dokie: %s\n", curl_easy_strerror(res));
          our_cleanup(ssl, curl, ctx, sock);
          exit(1);
        } else {
          if (reader.parse(str, js)) {
            std::cout << "City:\t\tBendzin" << std::endl;
            if (js.get("latitude", NAN).isDouble()) {
              std::cout << "Latitude:\t" << js.get("latitude", NAN).asDouble() <<
                std::endl;
            }
            if (js.get("longitude", NAN).isDouble()) {
              std::cout << "Longitude:\t" << js.get("longitude", NAN).asDouble() <<
                std::endl;
            }
            if (js.get("current_weather", "null").isObject()) {
              if (js.get("current_weather", "null").get("temperature",
                  NAN).isDouble()) {
                std::cout << "Temperature:\t" << js.get("current_weather",
                  "null").get("temperature", NAN).asDouble() << std::endl;
              }
              if (js.get("current_weather", "null").get("windspeed", NAN).isDouble()) {
                std::cout << "Wind speed:\t" << js.get("current_weather",
                  "null").get("windspeed", NAN).asDouble() << std::endl;
              }
              if (js.get("current_weather", "null").get("winddirection",
                  NAN).isDouble()) {
                std::cout << "Wind direction:\t" << js.get("current_weather",
                  "null").get("winddirection", NAN).asDouble() << std::endl;
              }
              if (js.get("current_weather", "null").get("weathercode",
                  NAN).isDouble()) {
                std::cout << "Weather code:\t" << js.get("current_weather",
                  "null").get("weathercode", NAN).asDouble() << std::endl;
              }
              std::cout << "Time:\t\t" << js.get("current_weather",
                "null").get("time", "null") << std::endl;
            }
            if (SSL_connect(ssl) < 0) {
              ERR_print_errors_fp(stderr);
              our_cleanup(ssl, curl, ctx, sock);
              exit(1);
            } else {
              printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
              if (SSL_write(ssl, str.c_str(), str.length()) <= 0) {
                printf("Blad w wysylaniu write 1");
                ERR_print_errors_fp(stderr);
                our_cleanup(ssl, curl, ctx, sock);
                exit(1);
              }
              our_cleanup(ssl, curl, ctx, sock);
            }
          }
          our_cleanup(ssl, curl, ctx, sock);
        }
      }
      our_cleanup(ssl, curl, ctx, sock);
      return 0;
    }
    std::cout << "Time zone:\t" << js.get("timezone", "null") << std::endl;
    return 0;
