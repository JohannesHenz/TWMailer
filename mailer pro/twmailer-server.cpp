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
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <regex>
#include <cstring>


#define BUF 1024
#define PORT 6543

int abortRequested = 0;
int create_socket = -1;
int new_socket = -1;

void *clientCommunication(void *data);
void signalHandler(int sig);

void saveMessage(const std::string& sender, const std::string& recipient, const std::string& subject, const std::string& message) {
    //finde msgnumber raus
    int index = 0;
    std::string folderName = "textlogs";
    std::string indexFileName = folderName + "/index.txt";
    std::ifstream indexFile1(indexFileName);
    if (indexFile1.is_open()) {
        indexFile1 >> index;
        indexFile1.close();
    } else {
        std::cerr << "Failed to read the index." << std::endl;
    }
    //erhoehe um 1 und schreib wieder rein, (std::ios::trunc verursacht hierbei, dass der alte index hierbei geloescht wird)
    index++;
    std::ofstream indexFile2(indexFileName, std::ios::trunc);
    if (indexFile2.is_open()) {
        indexFile2 << index;
        indexFile2.close();
    } else {
        std::cerr << "Failed to save the index." << std::endl;
    }
    //now we can create the txt file for the message
    std::string indexStr = std::to_string(index);
    std::string filePath = folderName + "/" + indexStr + "_" + sender + "_" + recipient +".txt"; //filename inkludiert

    std::string mail = subject + "\n" + message; //Die text files haben in der ersten zeile das subject und darunter den textkoerper
    // Write the message to a file
    std::ofstream outputFile(filePath);
    if (outputFile.is_open()) {
        outputFile << mail;
        outputFile.close();
    } else {
        std::cerr << "Failed to save the message." << std::endl;
    }
}

std::string GetList( [[maybe_unused]] const std::string& username, [[maybe_unused]] const std::string& listUser){ //[[maybe_unused]] wurde zwischenzeitlich gebraucht, damit der compiler nicht heult und jez isses halt da ¯\_(ツ)_/¯
    std::string StringfolderPath = "textlogs";
    const char* folderPath = StringfolderPath.c_str();  //opendir scheint nur mit const char arrays zu gehen
    int msgCount = 0;
    std::string list = "";
    DIR* dir = opendir(folderPath);
    if (!dir) {
        std::cerr << "Error opening directory" << std::endl;
        return "";
    }
    struct dirent* entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG) { //checks if it's a regular file and not a folder
            std::string fileName = entry->d_name;
            size_t pos = fileName.find_first_of('_');
            std::string part1 = fileName.substr(0, pos); // das ist der index
            fileName = fileName.substr(pos + 1); //nummer und _ wird vorne abgeschnitten
            pos = fileName.find_first_of('_');
            std::string sender = fileName.substr(0, pos); //das ist der name des absenders
            fileName = fileName.substr(pos + 1); //nummer und _ wird vorne abgeschnitten
            pos = fileName.find_first_of('.');
            std::string recipient = fileName.substr(0, pos); //das ist der name des empfängers
            std::cout << sender << std::endl;
            std::cout << recipient << std::endl;
            if(sender == username && recipient == listUser) {             // Check if the filename matches the expected pattern
                std::string filePath = folderPath;                         // schreib den pfad des files in filePath
                filePath += "/";
                filePath += entry->d_name;

                FILE* file = fopen(filePath.c_str(), "r");
                if (file) {
                    char buffer[1024];
                    if (fgets(buffer, sizeof(buffer), file)) {      //auslesen der ersten zeile, das passiert nur wenn da auch was steht
                    std::string StrBuffer(buffer); //cast auf string
                        list = list + StrBuffer + "\n";             //zur liste hinzufügen
                        ++msgCount;                                   //die messages zählen
                    }
                    fclose(file);
                }
            }
        }
    }

    closedir(dir);

    std::string output = "There are " +   std::to_string(msgCount) + " messages available:\n" + list;
    std::cout << output << std::endl; 
    return output; 
}

std::string ReadAndReply( [[maybe_unused]] const std::string& input_string) {
    const std::string folder_path = "textlogs/";
    std::string matching_file;

    DIR* dir = opendir(folder_path.c_str()); //hier finden wir erstmal den genauen filename raus
    if (dir != nullptr) { //also falls erfolgreich geöffnet
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) { //solange einträge im directory
            std::string file_name = entry->d_name; //schreib den namen in file_name

            if (file_name.find(input_string + "_") == 0) { //find gibt die stelle des ersten Vorkommen des Argumentes an, also is die id+ "_" am anfang falls das 0 ist
                matching_file = file_name;
                break;
            }
        }
        closedir(dir);
    }

    if (matching_file.empty()) { //falls es die nachricht nicht gibt
        return "Message not found";
    }

    std::string file_path = folder_path + matching_file;


    FILE* file = fopen(file_path.c_str(), "r"); //file öffnen mit lesezugriff
    if (file == nullptr) {
        // Error opening file
        return "Error opening file";
    }

    char buffer[1000000];
    size_t bytesRead = fread(buffer, 1, sizeof(buffer), file); //in bytes read den inhalt schreiben
    fclose(file);

    if (bytesRead == 0) { //es muss ein fehler aufgetreten, falls bytesRead hier immer noch leer ist
        return "Error reading file";
    }

    return std::string(buffer, bytesRead); //sonst auf string casten und raus damit
}

std::string DeleteMessage(const std::string& input_string) {
    const std::string folder_path = "textlogs/";
    std::string matching_file;

    DIR* dir = opendir(folder_path.c_str()); //das rennt recht analog zu ReadAndReply
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string file_name = entry->d_name;

            if (file_name.find(input_string + "_") == 0) {
                matching_file = file_name;
                break;
            }
        }
        closedir(dir);
    }

    if (!matching_file.empty()) {
        std::string file_path = folder_path + matching_file;

        // Try to delete the file
        if (std::remove(file_path.c_str()) == 0) {
            return "Message deleted";
        } else {
            return "Error deleting file";
        }
    } else {
        return "Message not found";
    }
}


void *clientCommunication(void *data){
   char buffer[BUF];
   int size;
   int *current_socket = (int *)data;

   strcpy(buffer, "Welcome to myserver!\r\nPlease enter your username:\r\n");
   if (send(*current_socket, buffer, strlen(buffer), 0) == -1)
   {
      perror("send failed");
      return NULL;
   }

    int bytesReceived = recv(*current_socket, buffer, strlen(buffer), 0); //schreibt den username der eingeben wurde in buffer
    buffer[bytesReceived] = '\0';

    std::string username = buffer; //username wird gesetzt
    strcpy(buffer, "Please enter your command:\r\n");
   if (send(*current_socket, buffer, strlen(buffer), 0) == -1){
      perror("send failed");
      return NULL;
   }

   do
   {
      size = recv(*current_socket, buffer, BUF-1, 0); //hier bekommt der server die msg
      if (size == -1){                                      //dann error handling
         if (abortRequested){
            perror("recv error after aborted");
         }
         else{
            perror("recv error");
         }
         break;
      }
      if (size == 0){
         printf("Client closed remote socket\n"); // ignore error
         break;
      }
      // remove ugly debug message, because of the sent newline of client
      if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n'){
         size -= 2;
      }
      else if (buffer[size - 1] == '\n'){
         --size;
      }
    //here we can finally act on the command
      buffer[size] = '\0';
      printf("Command received: %s\n", buffer); // ignore error

    if(strcmp(buffer, "SEND") == 0){
    //in diesem case wird basically der code von myserver.c reinkopiert. 
                    strcpy(buffer, "Who do you want to mail?\r\n");
                        if (send(*current_socket, buffer, strlen(buffer), 0) == -1)
                        {
                            perror("send failed");
                            return NULL;
                        }
                        int bytesReceived = recv(*current_socket, buffer, strlen(buffer), 0); //schreibt den username der eingeben wurde in buffer
                        buffer[bytesReceived] = '\0';
                        std::string recipient = buffer; //recipient wird gesetzt
                    strcpy(buffer, "What's the subject of the mail?\r\n");
                        if (send(*current_socket, buffer, strlen(buffer), 0) == -1)
                        {
                            perror("send failed");
                            return NULL;
                        }
                        bytesReceived = recv(*current_socket, buffer, strlen(buffer), 0); //schreibt den username der eingeben wurde in buffer
                        buffer[bytesReceived] = '\0';
                        std::string subject = buffer; //subject wird gesetzt
                    strcpy(buffer, "Please enter your message:\r\n");
                    if (send(*current_socket, buffer, strlen(buffer), 0) == -1){
                        perror("send failed");
                        return NULL;
                    }
                        size = recv(*current_socket, buffer, BUF - 1, 0);
                        if (size == -1)
                        {
                            if (abortRequested)
                            {
                                perror("recv error after aborted");
                            }
                            else
                            {
                                perror("recv error");
                            }
                            break;
                        }

                        if (size == 0)
                        {
                            printf("Client closed remote socket\n"); // ignore error
                            break;
                        }

                        // remove ugly debug message, because of the sent newline of client
                        if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n')
                        {
                            size -= 2;
                        }
                        else if (buffer[size - 1] == '\n')
                        {
                            --size;
                        }

                        buffer[size] = '\0';
                        saveMessage(username, recipient, subject, buffer); //recipient is hier derweil noch "server"
                        printf("Message received: %s\n", buffer); // ignore error

                        if (send(*current_socket, "OK", 3, 0) == -1)
                        {
                            perror("send answer failed");
                            return NULL;
                        }
            }
        else if(strcmp(buffer, "LIST") == 0){
                        strcpy(buffer, "Whose messages you received do you want listed?\r\n");
                        if (send(*current_socket, buffer, strlen(buffer), 0) == -1)
                        {
                            perror("send failed");
                            return NULL;
                        }
                        int bytesReceived = recv(*current_socket, buffer, strlen(buffer), 0); //schreibt den username der eingeben wurde in buffer
                        buffer[bytesReceived] = '\0';
                        std::string listUser = buffer; //listUser wird gesetzt
                        strcpy(buffer, GetList(username, listUser).c_str());
                        if (send(*current_socket, buffer, strlen(buffer), 0) == -1){
                            perror("send answer failed");
                            return NULL;
                        }
        }
        else if(strcmp(buffer, "READ") == 0){
                        strcpy(buffer, "Which message (by ID number) do you want to read?\r\n");
                        if (send(*current_socket, buffer, strlen(buffer), 0) == -1)
                        {
                            perror("send failed");
                            return NULL;
                        }
                        int bytesReceived = recv(*current_socket, buffer, strlen(buffer), 0); //schreibt die nummer in buffer
                        buffer[bytesReceived] = '\0';
                        std::string msgIndex = buffer; //msgIndex wird gesetzt
                        strcpy(buffer, ReadAndReply(msgIndex).c_str());
                        if (send(*current_socket, buffer, strlen(buffer), 0) == -1){
                            perror("send answer failed");
                            return NULL;
                        }
        }
        else if(strcmp(buffer, "DEL") == 0){
                        strcpy(buffer, "Which message (by ID number) do you want to delete?\r\n");
                        if (send(*current_socket, buffer, strlen(buffer), 0) == -1)
                        {
                            perror("send failed");
                            return NULL;
                        }
                        int bytesReceived = recv(*current_socket, buffer, strlen(buffer), 0); //schreibt die nummer in buffer
                        buffer[bytesReceived] = '\0';
                        std::string msgIndex = buffer; //msgIndex wird gesetzt
                        strcpy(buffer, DeleteMessage(msgIndex).c_str());
                        if (send(*current_socket, buffer, strlen(buffer), 0) == -1){
                            perror("send answer failed");
                            return NULL;
                        }
        }
        else if(strcmp(buffer, "QUIT") == 0){
                    strcpy(buffer, "Connection with server ended.\r\n");
                        if (send(*current_socket, buffer, strlen(buffer), 0) == -1)
                        {
                            perror("send failed");
                            return NULL;
                        }
        }//
        else{
                    if (send(*current_socket, "not a legal command", 20, 0) == -1){
                    perror("send answer failed");
                    return NULL;
                    }
        }
   } while (strcmp(buffer, "QUIT") != 0);

   // closes/frees the descriptor if not already
   if (*current_socket != -1)
   {
      if (shutdown(*current_socket, SHUT_RDWR) == -1)
      {
         perror("shutdown new_socket");
      }
      if (close(*current_socket) == -1)
      {
         perror("close new_socket");
      }
      *current_socket = -1;
   }

   return NULL;
}

void signalHandler(int sig)
{
   if (sig == SIGINT)
   {
      printf("abort Requested... "); // ignore error
      abortRequested = 1;
      if (new_socket != -1)
      {
         if (shutdown(new_socket, SHUT_RDWR) == -1)
         {
            perror("shutdown new_socket");
         }
         if (close(new_socket) == -1)
         {
            perror("close new_socket");
         }
         new_socket = -1;
      }

      if (create_socket != -1)
      {
         if (shutdown(create_socket, SHUT_RDWR) == -1)
         {
            perror("shutdown create_socket");
         }
         if (close(create_socket) == -1)
         {
            perror("close create_socket");
         }
         create_socket = -1;
      }
   }
   else
   {
      exit(sig);
   }
}

///////////////////////////////////////////////////////////////////////////////

int main(void)
{
   socklen_t addrlen;
   struct sockaddr_in address, cliaddress;
   int reuseValue = 1;

   if (signal(SIGINT, signalHandler) == SIG_ERR)
   {
      perror("signal can not be registered");
      return EXIT_FAILURE;
   }

   if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("Socket error"); // errno set by socket()
      return EXIT_FAILURE;
   }

   if (setsockopt(create_socket,
                  SOL_SOCKET,
                  SO_REUSEADDR,
                  &reuseValue,
                  sizeof(reuseValue)) == -1)
   {
      perror("set socket options - reuseAddr");
      return EXIT_FAILURE;
   }

   if (setsockopt(create_socket,
                  SOL_SOCKET,
                  SO_REUSEPORT,
                  &reuseValue,
                  sizeof(reuseValue)) == -1)
   {
      perror("set socket options - reusePort");
      return EXIT_FAILURE;
   }

   memset(&address, 0, sizeof(address));
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(PORT);

   if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1)
   {
      perror("bind error");
      return EXIT_FAILURE;
   }

   if (listen(create_socket, 5) == -1)
   {
      perror("listen error");
      return EXIT_FAILURE;
   }

   while (!abortRequested)
   {
      printf("Waiting for connections...\n");

      addrlen = sizeof(struct sockaddr_in);
      if ((new_socket = accept(create_socket,
                               (struct sockaddr *)&cliaddress,
                               &addrlen)) == -1)
      {
         if (abortRequested)
         {
            perror("accept error after aborted");
         }
         else
         {
            perror("accept error");
         }
         break;
      }

      printf("Client connected from %s:%d...\n",
             inet_ntoa(cliaddress.sin_addr),
             ntohs(cliaddress.sin_port));
      clientCommunication(&new_socket); // returnValue can be ignored
      new_socket = -1;
   }

   // frees the descriptor
   if (create_socket != -1)
   {
      if (shutdown(create_socket, SHUT_RDWR) == -1)
      {
         perror("shutdown create_socket");
      }
      if (close(create_socket) == -1)
      {
         perror("close create_socket");
      }
      create_socket = -1;
   }

   return EXIT_SUCCESS;
}


