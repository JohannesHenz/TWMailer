#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <regex>
#include <cstring>

void *clientCommunication(void *data);

void signalHandler(int sig);

void SEND();

void LIST();

void READ();

void DEL();

void QUIT();

void LDAPConnect();

int main()
{
    std::string hello = "sup";
    std::cout << hello << std::endl;
    return 0;
}