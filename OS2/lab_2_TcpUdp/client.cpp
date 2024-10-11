#include <iostream>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {

  // Describes IPv4 address
  struct sockaddr_in server_addr;
  
  memset(&server_addr, 0, sizeof(sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(9000);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(sockaddr_in)) < 0) {
    std::cerr << "Connect error\n";
    std::exit(1);
  }
  
  char msg_buf[1024];
  for(int i=0; i<10; i++) {
    int send_status = send(sockfd, "Hello", 5, 0);
    memset(msg_buf, 0, 1024);
    int recv_status = recv(sockfd, msg_buf, 1024, 0);
    std::cout << "Message from server: " << msg_buf << "\n";
    sleep(1);
  }

  int send_status = send(sockfd, "stop", 4, 0);
  
  close(sockfd);

  return 0;
}
