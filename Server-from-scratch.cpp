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
int newsocket = -1;
int StopEverything = 0;

#define BUF 1024
#define PORT 6543

void *clientCommunication(void *data);

void signalHandler(int sig);

void LOGIN();

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

  //Creating the Socket Descriptor
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  //Checking that the Address isn't already in use and setting the options
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseValue,
                 sizeof(reuseValue)) == -1) {
    perror("Adress already in Use");
    exit(EXIT_FAILURE);
  }

  //same thing with the Port
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuseValue,
                 sizeof(reuseValue)) == -1) {
    perror("Port already in Use");
    exit(EXIT_FAILURE);
  }

   memset(&serverAddress, 0, sizeof(serverAddress));
   serverAddress.sin_family = AF_INET;
   serverAddress.sin_addr.s_addr = INADDR_ANY;
   serverAddress.sin_port = htons(PORT);

  //binding the Socket Descriptor
  if (bind(sockfd, (struct sockaddr *)&serverAddress,sizeof(serverAddress)) == -1)
  {
    perror("Binding Error");
    exit(EXIT_FAILURE);
  }

  //listening for Clients
  if (listen(sockfd, 10) == -1)
  {
    perror("Listening Error");
    exit(EXIT_FAILURE);
  }

   while (!StopEverything)
   {
      /////////////////////////////////////////////////////////////////////////
      // ignore errors here... because only information message
      // https://linux.die.net/man/3/printf
      printf("Waiting for connections...\n");

      /////////////////////////////////////////////////////////////////////////
      // ACCEPTS CONNECTION SETUP
      // blocking, might have an accept-error on ctrl+c
      addrlen = sizeof(struct sockaddr_in);
      if ((newsocket = accept(sockfd,
                               (struct sockaddr *)&clientAddress,
                               &addrlen)) == -1)
      {
         if (StopEverything)
         {
            perror("accept error after aborted");
         }
         else
         {
            perror("accept error");
         }
         break;
      }

      /////////////////////////////////////////////////////////////////////////
      // START CLIENT
      // ignore printf error handling
      printf("Client connected from %s:%d...\n",
             inet_ntoa(clientAddress.sin_addr),
             ntohs(clientAddress.sin_port));
      clientCommunication(&newsocket); // returnValue can be ignored
      newsocket = -1;
   }
  


  return 0;
}

void signalHandler(int sig)
{
  if (sig == SIGINT)
   {
      printf("abort Requested... "); // ignore error
      StopEverything = 1;
      /////////////////////////////////////////////////////////////////////////
      // With shutdown() one can initiate normal TCP close sequence ignoring
      // the reference count.
      // https://beej.us/guide/bgnet/html/#close-and-shutdownget-outta-my-face
      // https://linux.die.net/man/3/shutdown
      if (newsocket != -1)
      {
         if (shutdown(newsocket, SHUT_RDWR) == -1)
         {
            perror("shutdown newsocket");
         }
         if (close(newsocket) == -1)
         {
            perror("close newsocket");
         }
         newsocket = -1;
      }

      if (sockfd != -1)
      {
         if (shutdown(sockfd, SHUT_RDWR) == -1)
         {
            perror("shutdown sockfd");
         }
         if (close(sockfd) == -1)
         {
            perror("close sockfd");
         }
         sockfd = -1;
      }
   }
  else
  {
  exit(sig);
  }
}