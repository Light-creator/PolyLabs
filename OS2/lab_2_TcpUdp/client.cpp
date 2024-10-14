#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MSG_LEN     1024

void form_messages(std::vector<std::string>& msgs, std::string& filename) {
  std::ifstream file(filename);
  
  const char* delimeters = " .\n";
  std::string str;

  int msg_idx = 0;
  while(std::getline(file, str)) {
    uint32_t date = 0, num2 = 0, msg_len = 0, net_msg_len = 0;
    int16_t num1 = 0;
    if(str.size() == 0) continue;
    
    uint32_t idx = 0;
    std::string msg;
    uint32_t net_msg_idx = htonl(msg_idx);
    msg.append((const char*)&net_msg_idx, sizeof(uint32_t));

    char* tk = std::strtok(&str[0], delimeters);
    while(tk) {
      switch(idx) {
        case 0:
          date = atoi(tk);
          break;
        case 1:
          date += atoi(tk)*100;
          break;
        case 2:
          date += atoi(tk)*10000;
          date = htonl(date);
          msg.append((const char*)&date, sizeof(uint32_t));
          break;
        case 3:
          num1 = atoi(tk);
          num1 = htons(num1);
          msg.append((const char*)&num1, sizeof(int16_t));
          break;
        case 4:
          num2 = atol(tk);
          num2 = htonl(num2);
          msg.append((const char*)&num2, sizeof(uint32_t));
          break;
        case 5:
          msg_len = strlen(tk);
          net_msg_len = htonl(msg_len);
          msg.append((const char*)&net_msg_len, sizeof(uint32_t));
          msg.append((const char*)tk, msg_len);
          break;
        default: break;
      }
      
      msgs.push_back(msg);
      idx++;
      tk = std::strtok(nullptr, delimeters);
    }
  }
}

int main() {
  // Describes IPv4 address
  struct sockaddr_in server_addr;
  std::string filename = "input1.txt";
  
  memset(&server_addr, 0, sizeof(sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(9000);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
  bool conn_success = false;
  for(int i=0; i<10; i++) {
    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
      conn_success = true;
      break;
    }

    usleep(100);
  }

  if(!conn_success) {
    std::cerr << "Connect error\n";
    std::exit(1);
  }

  char msg_buf[MSG_LEN];
  
  int send_status = send(sockfd, "put", 3, 0);
  int recv_status = recv(sockfd, msg_buf, 2, 0);
  
  std::vector<std::string> msgs;
  form_messages(msgs, filename);
  
  for(int i=0; i<10; i++) {
    send_status = send(sockfd, "Hello", 5, 0);
    memset(msg_buf, 0, MSG_LEN);
    recv_status = recv(sockfd, msg_buf, 2, 0);
    std::cout << "Message from server: " << msg_buf << "\n";
  }

  send_status = send(sockfd, "stop", 4, 0);
  
  close(sockfd);

  return 0;
}
