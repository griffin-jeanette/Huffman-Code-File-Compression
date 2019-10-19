/*
This program sends files to another computer through a socket connection
*/

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <stdint.h>
#include "sender.h"

int createSocket()
{
  int socketfd, newsocketfd, portno;
  struct sockaddr_in serv_addr, cli_addr;
  int clilen = sizeof(cli_addr);
  int n, opt = 1;

  if (0 > (socketfd = socket(AF_INET, SOCK_STREAM, 0)))
  {
    std::cout << "failed to create socket" << std::endl;
    exit(1);
  }

  if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
  {
    std::cout << "failed to set socket options" << std::endl;
    exit(1);
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);

  if (0 > bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)))
  {
    std::cout << "failed to bind socket" << std::endl;
    exit(1);
  }

  if (0 > listen(socketfd, 5))
  {
    std::cout << "listening error" << std::endl;
    exit(1);
  }

  if (0 > (newsocketfd = accept(socketfd, (struct sockaddr *) &cli_addr,(socklen_t*)&clilen)))
  {
    std::cout << "failed to accept" << std::endl;
    exit(1);
  }

  return newsocketfd;
}

/* receive a single packet of information */
void receiveData(int socketfd)
{
  char buffer[50];
  int bytesReceived = 0;

  if (0 > (bytesReceived = recv(socketfd, buffer, sizeof(buffer), 0)))
  {
    std::cout << "failed to receive buffer" << std::endl;
    exit(1);
  }
  buffer[bytesReceived] = '\0';
}

/* sends a string through a socket */
void sendDataStr(int socketfd, std::string & sendStr)
{
  if (0 > send(socketfd, sendStr.c_str(), strlen(sendStr.c_str()), 0))
  {
    std::cout << "failed to send string" << std::endl;
    exit(1);
  }
}

/* sends a message by first converting it to a
   particular string format:
      ((num bytes):(encoding),...,(num bytes):(encoding))*/
void sendData(int socketfd, Message message)
{
  std::string data = "(" + std::to_string(message.bytes) + ":" +
                     std::to_string(message.code) + ")";
  int errorCode;

  if (0 > (errorCode = send(socketfd, data.c_str(), strlen(data.c_str()), 0)))
  {
    std::cout << "failed to send data: " << errorCode << std::endl;
    exit(1);
  }
}
