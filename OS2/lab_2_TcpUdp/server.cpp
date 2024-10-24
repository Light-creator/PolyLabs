#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/poll.h>
#include <arpa/inet.h>

#include <fcntl.h>

#define MSG_LEN     1024
#define CLIENTS_LEN 64

union converter {
  char str[6];
  uint32_t u32;
  uint16_t u16;
  int16_t i16;
};

struct client_t {
  int fd;
  int port;
  std::string ip;
};

void parse_msg(client_t& client, char* msg_buf, std::ofstream& file) {
  char msg[MSG_LEN] = {0};
  converter conv;

  file << client.ip << ":" << client.port << " ";

  // Parse idx
  conv.str[0] = msg_buf[3]; conv.str[1] = msg_buf[2]; conv.str[2] = msg_buf[1]; conv.str[3] = msg_buf[0]; 
  // std::cout << "Message index: " << conv.u32 << "\n";
  // file << conv.u32 << " ";
  // Parse date
  conv.str[0] = msg_buf[7]; conv.str[1] = msg_buf[6]; conv.str[2] = msg_buf[5]; conv.str[3] = msg_buf[4];
  uint32_t date = conv.u32;
  int y = date / 10000;
  int m = date % 10000 / 100;
  int d = date % 100;
  // std::cout << "Date: " << d << "." << m << "." << y << "\n";
  file << d << "." << m << "." << y << " ";
  // Parse signed num
  conv.str[0] = msg_buf[9]; conv.str[1] = msg_buf[8];   
  // std::cout << "Signed num: " << conv.i16 << "\n";
  file << conv.i16 << " ";

  // Parse unsigned num
  conv.str[0] = msg_buf[13]; conv.str[1] = msg_buf[12]; conv.str[2] = msg_buf[11]; conv.str[3] = msg_buf[10];
  // std::cout << "Unsigned num: " << conv.u32 << "\n";
  file << conv.u32 << " ";

  // Parse text length
  conv.str[0] = msg_buf[17]; conv.str[1] = msg_buf[16]; conv.str[2] = msg_buf[15]; conv.str[3] = msg_buf[14];
  // std::cout << "Text length: " << conv.u32 << "\n";
  uint32_t len = conv.u32;

  // std::cout << "Text message: " << msg_buf+18 << "\n";
  // std::memcpy(msg_buf+18, msg, len);
  // std::cout << "msg: " << msg << " | " << msg_buf+18; 
  file << msg_buf+18 << "\n";
}



int main() {
  // Describes IPv4 address
  struct sockaddr_in server_addr, client_addr;
  int client_len = 0;

  std::string filename = "msgs.txt";
  std::ofstream file(filename);


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
  std::vector<client_t> clients;
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
          std::cout << "Client " << clients[i].fd << " closed connection...\n";
          close(clients[i].fd);
        }

        if(pfds[i].revents & POLLERR) {
          std::cout << "Client " << clients[i].fd << " error: close connection\n";
          close(clients[i].fd);
        }

        if(pfds[i].revents & POLLIN) {
          int recv_status = recv(clients[i].fd, msg_buf, MSG_LEN, 0);
          // std::cout << "Client " << clients[i].fd << " message: " << msg_buf << "\n";
          if(strncmp(msg_buf, "put", 3) == 0) {}
          else if(strncmp(msg_buf, "stop", 4) == 0) goto done;
          else parse_msg(clients[i], msg_buf, file);
          flag_recv = true;
        }

        if(pfds[i].revents & POLLOUT && flag_recv) {
          int send_status = send(clients[i].fd, "ok", 2, 0);
        }
      }

      if(pfds[CLIENTS_LEN].revents & POLLIN) {
        int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&client_len);
        
        char addr_buf[16];
        inet_ntop(AF_INET, &client_addr.sin_addr, addr_buf, sizeof(addr_buf));
        
        client_t client;

        client.fd = clientfd;
        client.ip = addr_buf;
        client.port = htons(client_addr.sin_port);

        std::cout << "Client: " << client.ip << ":" << client.port << " connected...\n";

        pfds[clients.size()].fd = clientfd;
        pfds[clients.size()].events = POLLIN | POLLOUT;

        clients.push_back(client);
      }
    } else {
      std::cout << "Timed out\n";
      break;
    }
  }

done:
  for(int i=0; i<clients.size(); i++) close(clients[i].fd);
  close(sockfd);

  return 0;
}
