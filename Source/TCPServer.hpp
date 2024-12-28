#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <functional>
#include <stdexcept>

#include "ThreadPool.hpp"

class TCPServer {
 public:
  using ClientHandler = std::function<void(int, const char*, std::size_t)>;

  void create() {
    m_serverFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_serverFd == -1) {
      throw std::runtime_error("Failed to create server socket");
    }

    int flags{SO_REUSEADDR | SO_REUSEPORT | SO_BROADCAST | SO_KEEPALIVE};
    int opt{1};
    setsockopt(m_serverFd, SOL_SOCKET, flags, &opt, sizeof(opt));

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(m_port);

    if (bind(m_serverFd, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == 1) {
      throw std::runtime_error("Failed to bind server socket");
    }

    setNonBlocking(m_serverFd);
    listen(m_serverFd, SOMAXCONN);

    m_epollFd = epoll_create1(0);
    if (m_epollFd == -1) {
      throw std::runtime_error("Failed to create epoll instance");
    }

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = m_serverFd;
    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_serverFd, &event) == -1) {
      throw std::runtime_error("Failed to add server socket to epoll instance");
    }
  };

  void run(ClientHandler handleClient) {
    while (true) {
      int eventNumbers = epoll_wait(m_epollFd, m_events, 1024, 100);

      for (int i = 0; i < eventNumbers; ++i) {
        if (m_events[i].data.fd == m_serverFd) {
          int clientFd = accept(m_serverFd, nullptr, nullptr);
          if (clientFd == -1) {
            continue;
          }

          epoll_event event;
          event.events = EPOLLIN;
          event.data.fd = clientFd;
          epoll_ctl(m_epollFd, EPOLL_CTL_ADD, clientFd, &event);
        } else {
          int clientFd = m_events[i].data.fd;
          char buffer[1024];
          std::size_t size = recv(clientFd, buffer, sizeof(buffer), 0);

          if (size > 0) {
            m_threadPool.push([=] { handleClient(clientFd, buffer, size); });
          } else {
            epoll_ctl(m_epollFd, EPOLL_CTL_DEL, clientFd, nullptr);
            close(clientFd);
          }
        }
      }
    }
  };

 private:
  void setNonBlocking(int socketFd) noexcept {
    int flags = fcntl(socketFd, F_GETFL, 0);
    fcntl(socketFd, F_SETFL, flags | O_NONBLOCK);
  }

  uint16_t m_port{8080};
  epoll_event m_events[1024];
  int m_serverFd;
  int m_epollFd;
  ThreadPool m_threadPool;
};
