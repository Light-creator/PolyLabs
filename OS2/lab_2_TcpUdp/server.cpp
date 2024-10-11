#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <fcntl.h>

#define MSG_LEN     1024
#define CLIENTS_LEN 64

int main() {
  // Describes IPv4 address
  struct sockaddr_in server_addr, client_addr;
  int client_len = 0;

  // int clients[CLIENTS_LEN] = {0};
  fd_set rfd;
  fd_set wfd;
  struct timeval tv = {10, 0};

  memset(&server_addr, 0, sizeof(sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(9000);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  int fl = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, fl | O_NONBLOCK);
  
  if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    std::cerr << "Bind error\n";
    std::exit(1);
  }

  if(listen(sockfd, 5) < 0) {
    std::cerr << "Listen error\n";
    std::exit(1);
  }
  
  int maxfd = sockfd;
  char msg_buf[MSG_LEN];
  std::vector<int> clients;
  for(;;) {
    // int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&client_len);
    // clients[clientfd] = 1;
    // std::cout << "Client connected...\n";
    
    // std::cout << "Set fds\n";
    FD_ZERO(&rfd);
    FD_ZERO(&wfd);
    FD_SET(sockfd, &rfd);

    for(int i=0; i<clients.size(); i++) {
      FD_SET(clients[i], &rfd);
      FD_SET(clients[i], &wfd);
      if(clients[i] > maxfd) maxfd = clients[i];
    }

    if(select(maxfd+1, &rfd, &wfd, 0, &tv) > 0) {
      if(FD_ISSET(sockfd, &rfd)) {
        int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&client_len);
        std::cout << "Client " << clientfd << " connected...\n";
        clients.push_back(clientfd);
      }

      for(int i=0; i<clients.size(); i++) {
        int recv_status = 0;
        if(FD_ISSET(clients[i], &rfd)) {
          recv_status = recv(clients[i], msg_buf, MSG_LEN, 0);
          std::cout << "Client " << clients[i] << ": " << msg_buf << "\n";

          if(strncmp(msg_buf, "stop", 4) == 0) {
            std::cout << "Client " << clients[i] << " disconnected...\n";
            clients.erase(clients.begin()+i);
          }
        }

        if(FD_ISSET(clients[i], &wfd) && recv_status) {
          int send_status = send(clients[i], "ok", 2, 0);
        }
      }
    } else {
      break;
    }
  }

  close(sockfd);

  return 0;
}
