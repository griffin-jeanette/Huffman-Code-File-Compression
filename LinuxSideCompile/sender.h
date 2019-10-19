/*
  header file for sender.cpp
*/

#ifndef SENDER_HPP
#define SENDER_HPP

#include <string>
#include <stdint.h>

const int PORT = 8080;

typedef struct Messages
{
  uint32_t bytes;
  uint32_t code;
} Message;

int createSocket();

void receiveData(int socketfd);

void sendDataStr(int socketfd, std::string & sendStr);

void sendData(int socketfd, Message message);

#endif
