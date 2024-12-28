
#include <arpa/inet.h>

#include <iostream>

#include "TCPServer.hpp"

void handleClient(int clientFd, const char* buffer, std::size_t size) {
  std::string str{buffer, size};
  send(clientFd, buffer, size, 0);
  close(clientFd);
};

int main() {
  TCPServer server;
  server.create();
  server.run(handleClient);
  return 0;
}
