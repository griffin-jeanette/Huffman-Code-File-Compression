#pragma once
/*
	header for wSender.cpp
*/

#ifndef WSENDER_HPP
#define WSENDER_HPP

#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <vector>
#include <unordered_map>
#include <string>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

const char PORT[8] = "8080";
const char IPADDRESS[16] = "140.141.132.43";

struct Message
{
	uint32_t bytes;
	uint32_t code;
};

SOCKET createSocket();

void receiveCodes(SOCKET ConnectSocket, std::vector<Message> & codes);

void receiveTree(SOCKET ConnectSocket, std::unordered_map<std::string, char> & counts);

void sendReady(SOCKET ConnectSocket);

void shut(SOCKET ConnectSocket);

#endif