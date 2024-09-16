#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>

#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <chrono>
#include <thread>

#include <unistd.h>

void error(const char* msg) {
  std::cerr << msg << "\n";
  std::exit(EXIT_FAILURE);
}

bool parse_server_addr(char* addr, char* ip, int& port) {
  int i=0;

  std::cout << "Addr: " << addr << "\n";
  while(i < 17 && *addr != ':' && *addr != '\0') {
    *ip++ = *addr++;
    i++;
  }
  
  *addr++;
  *ip = '\0';
  
  char port_str[6];
  int j = 0;
  while(*addr != '\0' && j < 6) {
    port_str[j++] = *addr++;
  }

  port_str[j] = '\0';
  port = atoi(port_str);
 
  return true;
}

void read_file(char* filename, std::vector<std::string>& v) {
  std::ifstream file(filename);
  if(!file.is_open()) error("Cannot open input file");
  
  std::string s;
  while(std::getline(file, s)) {
    v.push_back(s);
  }
}

void form_msg(std::string& s, std::string& msg) {
  msg.push_back('c');
}   

int send_request(int sockfd, char* filename) {
  char buf[2];
  int res_status = send(sockfd, "put", 3, MSG_NOSIGNAL);
  
  std::vector<std::string> v;
  read_file(filename, v);

  if(v.size() == 0) error("Cannot find messages");
  int count_receved = 0;
  
  uint32_t idx = 0;
  for(std::string& s: v) {
    std::string msg;
    form_msg(s, msg);
    res_status = send(sockfd, msg.c_str(), msg.size(), MSG_NOSIGNAL);

    while(recv(sockfd, buf, 2, 0)) {
      if(strcmp(buf, "ok") == 0) {
        count_receved++;
        break;
      }
    }
  }

  return 0;
}

int main(int argc, char** argv) {
  if(argc < 3 || argc > 3) {
    error("Usage: ./tcpclient <ip:port> <input_file>");
  }

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) error("Socket has not been created");
  
  char ip[17];
  int port;
  if(!parse_server_addr(argv[1], ip, port)) error("Incorrect address");
    
  std::cout << ip << " " << port << "\n";
 
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);
  
  for(int conn_try=0; conn_try<10; conn_try++) {
    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
      send_request(sockfd, argv[2]);
    } else {
      std::cout << "Trying to connect " << conn_try+1 << " attempt\n"; 
      usleep(100000);
      // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  
  close(sockfd);

  return 0;
}
