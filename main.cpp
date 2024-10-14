#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>

using namespace std;

// 最大连接数
const int MAX_CONN = 1024;

// 保存客户端信息
struct Client {
    int sockfd;
    string name;
};

int main() {
    int epld = epoll_create(1);
    if (epld < 0) {
        perror("epoll_create error");
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        close(epld);
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9999);

    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind error");
        close(sockfd);
        close(epld);
        return -1;
    }

    ret = listen(sockfd, MAX_CONN);
    if (ret < 0) {
        perror("listen error");
        close(sockfd);
        close(epld);
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    ret = epoll_ctl(epld, EPOLL_CTL_ADD, sockfd, &ev);
    if (ret < 0) {
        perror("epoll_ctl error");
        close(sockfd);
        close(epld);
        return -1;
    }

    map<int, Client> clients;

    while (1) {
        struct epoll_event evs[MAX_CONN];
        int n = epoll_wait(epld, evs, MAX_CONN, -1);
        if (n < 0) {
            perror("epoll_wait error");
            break;
        }

        for (int i = 0; i < n; i++) {
            int fd = evs[i].data.fd;
            if (fd == sockfd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
                if (client_sockfd < 0) {
                    perror("accept error");
                    continue;
                }

                struct epoll_event ev_client;
                ev_client.events = EPOLLIN;
                ev_client.data.fd = client_sockfd;
                if (epoll_ctl(epld, EPOLL_CTL_ADD, client_sockfd, &ev_client) < 0) {
                    perror("epoll_ctl error");
                    close(client_sockfd);
                    continue;
                }

                printf("%s 正在连接......\n", inet_ntoa(client_addr.sin_addr));
                clients[client_sockfd] = {client_sockfd, ""};
            }
            else {
                char buffer[1024];
                int n = read(fd, buffer, sizeof(buffer));
                if (n < 0) {
                    perror("read error");
                }
                else if (n == 0) {
                    close(fd);
                    epoll_ctl(epld, EPOLL_CTL_DEL, fd, NULL);
                    clients.erase(fd);
                    printf("客户端 %d 断开连接\n", fd);
                }
                else {
                    string msg(buffer, n);
                    if (clients[fd].name.empty()) {
                        clients[fd].name = msg;
                    }
                    else {
                        string name = clients[fd].name;
                        string full_msg = '[' + name + "] " + msg;
                        for (auto& c : clients) {
                            if (c.first != fd) {
                                write(c.first, full_msg.c_str(), full_msg.size());
                            }
                        }
                    }
                }
            }
        }
    }
    close(sockfd);
    close(epld);

    return 0;
}
