//This library is made by @Nullora on Github. The link can be found here, and documentation aswell: https://github.com/Nullora/NovusNet
//NovusNet is a c++ networking library made to facilitate connection between devices while keeping it fast and secure.
//It's fully free and anyone can distribute/use it.
//Last updated: 31/3/26
#include "nn.hpp"
#include <map>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <endian.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <algorithm>
#include <string>

// i seperated the send and recv cz its going south
struct ConnMutexes {
    std::mutex send_mtx;
    std::mutex recv_mtx;
};

static std::map<int, SSL*> s_clients;
static std::atomic<int> s_client_index{0};
static std::function<void(int, std::string)> s_messageCallback;
static SSL_CTX*  s_ssl_ctx    = nullptr;
static SSL* s_client_ssl = nullptr;
static std::mutex s_clients_mutex;
static std::map<int, std::shared_ptr<ConnMutexes>> s_conn_mutexes;
static ConnMutexes s_client_conn_mutexes; // client side


static SSL* getSSL(int id) {
    std::lock_guard<std::mutex> lk(s_clients_mutex);
    auto it = s_clients.find(id);
    return it != s_clients.end() ? it->second : s_client_ssl;
}

static std::shared_ptr<ConnMutexes> getConn(int id) {
    std::lock_guard<std::mutex> lk(s_clients_mutex);
    auto it = s_conn_mutexes.find(id);
    return it != s_conn_mutexes.end() ? it->second : nullptr;
}


//sendraw and recv so i can call it from functions that lock the mutex.
static bool _sendRaw(SSL* ssl, const std::string& msg) {
    uint32_t len_net = htonl((uint32_t)msg.size());
    const char* hdr  = reinterpret_cast<const char*>(&len_net);
    int hdr_left = (int)sizeof(len_net), hdr_sent = 0;
    while (hdr_left > 0) {
        int r = SSL_write(ssl, hdr + hdr_sent, hdr_left);
        if (r <= 0) return false;
        hdr_sent += r;
        hdr_left -= r;
    }
    int left = (int)msg.size(), sent = 0;
    while (left > 0) {
        int r = SSL_write(ssl, msg.c_str() + sent, left);
        if (r <= 0) return false;
        sent += r;
        left -= r;
    }
    return true;
}

static std::string _recvRaw(SSL* ssl) {
    uint32_t len_net = 0;
    int hdr_left = (int)sizeof(len_net), hdr_recv = 0;
    while (hdr_left > 0) {
        int r = SSL_read(ssl, reinterpret_cast<char*>(&len_net) + hdr_recv, hdr_left);
        if (r <= 0) return "EXITED(C-178)";
        hdr_recv += r;
        hdr_left -= r;
    }
    int len = (int)ntohl(len_net);
    if (len <= 0 || len > 4 * 1024 * 1024) return "EXITED(C-178)";
    std::string msg(len, '\0');
    int left = len, recvd = 0;
    while (left > 0) {
        int r = SSL_read(ssl, msg.data() + recvd, left);
        if (r <= 0) return "EXITED(C-178)";
        recvd += r;
        left -= r;
    }
    return msg;
}

static auto printProgress = [](uint64_t done, uint64_t total) {
    int pct = (int)((done * 100) / total);
    int filled = pct / 5;
    std::cout << "\r[";
    for (int i = 0; i < 20; ++i) std::cout << (i < filled ? '#' : '-');
    std::cout << "] " << pct << "% " << std::flush;
};


void onMessage(std::function<void(int, std::string)> callback) {
    s_messageCallback = callback;
}

//NMTS
bool sendMsg(std::string msg, int id) {
    SSL* ssl = getSSL(id);
    if (!ssl) return false;
    auto conn = getConn(id);
    if (conn) {
        std::lock_guard<std::mutex> lk(conn->send_mtx);
        return _sendRaw(ssl, msg);
    }

    std::lock_guard<std::mutex> lk(s_client_conn_mutexes.send_mtx);
    return _sendRaw(ssl, msg);
}


std::string recvMsg(int id) {
    SSL* ssl = getSSL(id);
    if (!ssl) return "EXITED(C-178)";
    auto conn = getConn(id);
    if (conn) {
        std::lock_guard<std::mutex> lk(conn->recv_mtx);
        return _recvRaw(ssl);
    }
    std::lock_guard<std::mutex> lk(s_client_conn_mutexes.recv_mtx);
    return _recvRaw(ssl);
}

void runServer(int port, std::string password) {
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) ||
        !SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM)) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return;
    }
    s_ssl_ctx = ctx;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return; }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(server_fd,(sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind");   return; }
    if (listen(server_fd, 32)< 0) { perror("listen"); return; }
    std::cout << "Server on port " << port << "\n";

    std::thread([server_fd, password]() {
        while (true) {
            sockaddr_in client_addr{};
            socklen_t clen = sizeof(client_addr);
            int new_fd = accept(server_fd, (sockaddr*)&client_addr, &clen);
            if (new_fd < 0) continue;

            SSL* ssl = SSL_new(s_ssl_ctx);
            SSL_set_fd(ssl, new_fd);
            if (SSL_accept(ssl) <= 0) {
                ERR_print_errors_fp(stderr);
                SSL_free(ssl);
                close(new_fd);
                continue;
            }

            int ci    = ++s_client_index;
            auto conn = std::make_shared<ConnMutexes>();
            {
                std::lock_guard<std::mutex> lk(s_clients_mutex);
                s_clients[ci] = ssl;
                s_conn_mutexes[ci] = conn;
            }

            //receive password under the recv mutex.
            std::string recvdp;
            {
                std::lock_guard<std::mutex> lk(conn->recv_mtx);
                recvdp = _recvRaw(ssl);
            }

            if (recvdp != password) {
                std::cout << "KICKED (wrong password): " << inet_ntoa(client_addr.sin_addr) << "\n";
                {
                    std::lock_guard<std::mutex> lk(s_clients_mutex);
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                    s_clients.erase(ci);
                    s_conn_mutexes.erase(ci);
                }
                close(new_fd);
            } else {
                std::cout << "CONNECTED: " << inet_ntoa(client_addr.sin_addr) << "\n";
                {
                    std::lock_guard<std::mutex> lk(conn->send_mtx);
                    _sendRaw(ssl, std::to_string(ci));
                }

                std::thread([ci, ssl, new_fd, conn]() {
                    while (true) {
                        std::string msg;
                        {
                            std::lock_guard<std::mutex> lk(conn->recv_mtx);
                            msg = _recvRaw(ssl);
                        }
                        if (msg == "EXITED(C-178)") {
                            std::cout << "client " << ci << " disconnected, cleaning.\n";
                            {
                                std::lock_guard<std::mutex> lk(s_clients_mutex);
                                SSL_shutdown(ssl);
                                SSL_free(ssl);
                                s_clients.erase(ci);
                                s_conn_mutexes.erase(ci);
                            }
                            close(new_fd);
                            break;
                        }
                        //record the actual message
                        if (s_messageCallback) s_messageCallback(ci, msg);
                    }
                }).detach();
            }
        }
    }).detach();
}

int runClient(std::string ip, int port, std::string password) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return -1; }

    sockaddr_in srv{};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &srv.sin_addr);

    if (connect(fd, (sockaddr*)&srv, sizeof(srv)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }
    //finding cert
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
    SSL_CTX_load_verify_locations(ctx, "cert.pem", nullptr);
    SSL* ssl = SSL_new(ctx);
    SSL_CTX_free(ctx);
    SSL_set_fd(ssl, fd);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(fd);
        return -1;
    }
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        std::cerr << "Certificate verification failed\n";
        SSL_free(ssl);
        close(fd);
        return -1;
    }

    s_client_ssl = ssl;
    {
        std::lock_guard<std::mutex> lk(s_client_conn_mutexes.send_mtx);
        _sendRaw(ssl, password);
    }
    std::cout << "Request sent\n";
    return fd;
}

//NFTP
bool sendFile(std::string filepath, int id) {
    SSL* ssl = getSSL(id);
    if (!ssl) return false;
    auto conn = getConn(id);

    std::mutex& send_mtx = conn ? conn->send_mtx : s_client_conn_mutexes.send_mtx;
    std::lock_guard<std::mutex> lk(send_mtx);

    int fd = open(filepath.c_str(), O_RDONLY);
    if (fd < 0) { perror("sendFile: open"); return false; }

    struct stat st;
    fstat(fd, &st);
    uint64_t size    = (uint64_t)st.st_size;
    uint64_t net_sz  = htobe64(size);

    if (SSL_write(ssl, &net_sz, sizeof(net_sz)) <= 0) { close(fd); return false; }

    std::string filename = filepath.substr(filepath.find_last_of("/\\") + 1);
    if (!_sendRaw(ssl, filename)) { close(fd); return false; }

    char buf[16384];
    uint64_t total_sent = 0;
    while (total_sent < size) {
        ssize_t rd = read(fd, buf, sizeof(buf));
        if (rd <= 0) break;
        int off = 0;
        while (off < (int)rd) {
            int sent = SSL_write(ssl, buf + off, (int)rd - off);
            if (sent <= 0) { close(fd); return false; }
            off += sent;
            total_sent += (uint64_t)sent;
        }
        printProgress(total_sent, size);
    }
    std::cout << "\n";
    close(fd);
    return total_sent == size;
}

//now calls recvraw so it doesnt fuck up the mutex
bool recvFile(std::string folderpath, int id) {
    SSL* ssl = getSSL(id);
    if (!ssl) return false;
    auto conn = getConn(id);

    std::mutex& recv_mtx = conn ? conn->recv_mtx : s_client_conn_mutexes.recv_mtx;
    std::lock_guard<std::mutex> lk(recv_mtx);

    uint64_t net_sz = 0;
    if (SSL_read(ssl, &net_sz, sizeof(net_sz)) <= 0) { perror("recvFile: size"); return false; }
    uint64_t filesize = be64toh(net_sz);
    if (filesize == 0 || filesize > 10ULL * 1024 * 1024 * 1024) return false;

    std::string filename = _recvRaw(ssl);
    if (filename.empty()                             ||
        filename == "EXITED(C-178)"                  ||
        filename.size() > 255                        ||
        filename.find('/')  != std::string::npos     ||
        filename.find('\\') != std::string::npos)
        return false;

    std::string fullpath = folderpath + "/" + filename;
    int outfd = open(fullpath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outfd < 0) { perror("recvFile: create"); return false; }

    char     buf[16384];
    uint64_t left = filesize, recvd = 0;
    while (left > 0) {
        int to_read = (int)std::min(left, (uint64_t)sizeof(buf));
        int r = SSL_read(ssl, buf, to_read);
        if (r <= 0) { close(outfd); return false; }
        if (write(outfd, buf, r) != r) {
            perror("recvFile: write");
            close(outfd);
            return false;
        }
        left  -= (uint64_t)r;
        recvd += (uint64_t)r;
        printProgress(recvd, filesize);
    }
    std::cout << "\n";
    close(outfd);
    return true;
}