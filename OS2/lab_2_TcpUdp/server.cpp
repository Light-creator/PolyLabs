#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/poll.h>

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
  // int clients[CLIENTS_LEN];
  
  // Configure pollfd array
  struct pollfd pfds[CLIENTS_LEN+1];
  pfds[CLIENTS_LEN].fd = sockfd;
  pfds[CLIENTS_LEN].events = POLLIN;
  
  for(;;) {
    int event = poll(pfds, sizeof(pfds)/sizeof(pfds[0]), 1000);
    if(event > 0) {
      for(int i=0; i<clients.size(); i++) {
        bool flag_recv = false;
   
        if(pfds[i].revents & POLLHUP) {
          close(clients[i]);
        }

        if(pfds[i].revents & POLLERR) {
          close(clients[i]);
        }

        if(pfds[i].revents & POLLIN) {
          int recv_status = recv(clients[i], msg_buf, MSG_LEN, 0);
          std::cout << "Client " << clients[i] << " message: " << msg_buf << "\n";
          if(strncmp(msg_buf, "stop", 4) == 0) goto done;
          flag_recv = true;
        }

        if(pfds[i].revents & POLLOUT && flag_recv) {
          int send_status = send(clients[i], "ok", 2, 0);
        }
      }

      if(pfds[CLIENTS_LEN].revents & POLLIN) {
        int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&client_len);
        std::cout << "Client: " << clientfd << " connected...\n";

        pfds[clients.size()].fd = clientfd;
        pfds[clients.size()].events = POLLIN | POLLOUT;

        clients.push_back(clientfd);
      }
    } else {
      std::cout << "Timed out\n";
      break;
    }
  }

done:
  for(int i=0; i<clients.size(); i++) close(clients[i]);
  close(sockfd);

  return 0;
}
