
#include "stdafx.h"
#include <stdlib.h>
#include <string>
#include "huffNode.h"
#include "wSender.h"
#include <vector>
#include <sstream>
#include <string>
#include <iostream>
using namespace std;

SOCKET createSocket()
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;
	int iResult;
	char sendBuf[10] = "testing";
	int count = 0;

	/* initialize Winsock */
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed with error: " << iResult << std::endl;
		exit(1);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	/* Resolve the server address and port */
	if (0 != (iResult = getaddrinfo(IPADDRESS, PORT, &hints, &result)))
	{
		std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
		WSACleanup();
		exit(1);
	}

	/* Attempt to connect to an address until one succeeds */
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		/* Create a SOCKET for connecting to server */
		if (INVALID_SOCKET == (ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)))
		{
			std::cout << "socket failed with error " << WSAGetLastError() << std::endl;
			WSACleanup();
			exit(1);
		}

		/* Connect to server */
		if (SOCKET_ERROR == (iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen)))
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET)
	{
		std::cout << "Unable to connect to server" << std::endl;
		WSACleanup();
		exit(1);
	}
	else
	{
		std::cout << "Connected to server successfully!" << std::endl;
		return ConnectSocket;
	}

	return 0;
}

void receiveCodes(SOCKET ConnectSocket, std::vector<Message> & codes)
{
	int iResult = 0;
	char inputStr[100];
	string total = "";
	string size = "";
	string code = "";
	int j = 0;

	/* get encoded file */
	do
	{
		iResult = recv(ConnectSocket, inputStr, 25, 0);
		if (iResult != 0)
		{
			inputStr[iResult] = '\0';
			total += inputStr;
		}

	} while (iResult != 0);

	/* read through all encodings of form: 
		((num bytes):(uint32_t)) */
	for (size_t i = 0; i < total.length(); i++)
	{
		// reached a new code
		if (total[i] == '(')
		{
			/* skip over '(' */
			i += 1;

			/* store off num bytes */
			while (total[i] != ':')
				size.push_back(total[i++]);

			j = 0;
			i += 1;

			/* store off number */
			while (total[i] != ')')
				code.push_back(total[i++]);

			j = 0;

			/* create new message containing number and number of
			   relevant bytes */
			Message newMessage;
			newMessage.bytes = strtoul(size.c_str(), NULL, 10);
			newMessage.code = strtoul(code.c_str(), NULL, 10);
			codes.push_back(newMessage);

			size = "";
			code = "";
		}
	}
}

void receiveTree(SOCKET ConnectSocket, unordered_map<string, char> & counts)
{
	int iResult = 0;
	char data[100];
	std::string count = "";
	std::string code = "";
	char letter;
	int f = 0;

	/* get tree header */
	do {
		iResult = recv(ConnectSocket, data, 98, 0);
		if (iResult > 0)
		{
			data[iResult] = '\0';
			count += data;
		}
		else if (iResult == 0)
			printf("Received all data\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());

	} while (data[iResult - 1] != ']');
	
	/* read through character codes of form:
		['char1':1010, 'char2':111,...,] */
	for (size_t i = 1; i < count.length(); i++)
	{
		letter = count[i];
		
		/* skip over ':' */
		i += 2;

		/* get encoding */
		while ((i < count.length() && (count[i] != ',')))
		{
			code += count[i++];
		}

		counts[code] = letter;
		code = "";
	}
}

/* signals to other end of socket to continue sending information */
void sendReady(SOCKET ConnectSocket)
{
	int iResult = 0;
	char sendBuf[10] = "ready";

	if (SOCKET_ERROR == (iResult = send(ConnectSocket, sendBuf, (int) strlen(sendBuf), 0)))
	{
		std::cout << "send failed with error: " << iResult << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		exit(1);
	}
}

/* shuts down a socket connection */
void shut(SOCKET ConnectSocket)
{
	int iResult = 0;
	if (SOCKET_ERROR == (iResult = shutdown(ConnectSocket, SD_SEND)))
	{
		std::cout << "shutdown failed with error: " << iResult << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		exit(1);
	}

	closesocket(ConnectSocket);
	WSACleanup();
}
