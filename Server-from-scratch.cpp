#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <regex>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

int sockfd = -1;

#define BUF 1024
#define PORT 6543

void *clientCommunication(void *data);

void signalHandler(int sig);

void SEND();

void LIST();

void READ();

void DEL();

void QUIT();

void LDAPConnect();

int main() {
  int reuseValue = -1;
  socklen_t addrlen;
  struct sockaddr_in serverAddress, clientAddress;


  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseValue,
                 sizeof(reuseValue)) == -1) {
    perror("Adress already in Use");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuseValue,
                 sizeof(reuseValue)) == -1) {
    perror("Port already in Use");
    exit(EXIT_FAILURE);
  }

  if (bind(sockfd, (struct sockaddr *)&serverAddress,sizeof(serverAddress)) == -1)
  {
    perror("bind error");
    exit(EXIT_FAILURE);
  }
  

  return 0;
}