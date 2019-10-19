// ConsoleApplication2.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "wSender.h"
#include "huffNode.h"
#include <stdint.h>
#include <vector>
#include <iostream>
#include <queue>
#include <intrin.h>
#include <string>
#include <fstream>
using namespace std;

/* custom compare struct used in the priority queue */
struct Compare
{
	bool operator() (HuffNode * node1, HuffNode * node2)
	{
		return node1->getFreq() > node2->getFreq();
	}
};

/* reads through a vector containing segments on the encoded file and 
   uses the Huffman Tree to decode the file */
void decode(HuffNode * root, vector<Message> & messages)
{
	HuffNode * node = root;
	uint32_t bit;
	uint32_t code;
	uint32_t mask;
	uint32_t size;
	char letter;
	ofstream myFile;

	myFile.open("test.txt");

	/* read through each message */
	for (size_t i = 0; i < messages.size(); i++)
	{
		code = messages[i].code;
		size = messages[i].bytes - 1;
		mask = 1ULL << ((unsigned long long) size);

		for (uint32_t j = 0; j < size + 1; j++)
		{
			/* we have reached a character in the tree */
			if ((node->left == NULL) && (node->right == NULL))
			{

				letter = node->getChar();
				if (letter == '%')
					myFile << '\n';
				else
					myFile << letter;
				node = root;
			}

			/* get next bit in encoding */
			bit = code & mask;
			bit >>= (((unsigned long) size) - j);
			mask >>= 1UL;

			/* traverse the tree */
			if (bit)
				node = node->right;
			else
				node = node->left;
		}
	}
	myFile.close();
}

void printTreeCodes(HuffNode * root, string str)
{
	if (!root)
		return;

	if (root->getChar() != '$')
	{
		std::cout << root->getChar() << ": " << str << std::endl;
	}

	printTreeCodes(root->left, str + "0");
	printTreeCodes(root->right, str + "1");
}

/* constructs a Huffman Tree based on already established encodings of characters*/
void constructTree(unordered_map<string, char> & codes, HuffNode * root)
{
	string encoding;
	HuffNode * temp = root;

	/* use character encodings to reverse engineer a Huffman Tree */
	for (auto item : codes)
	{
		encoding = item.first;

		for (size_t i = 0; i < encoding.length(); i++)
		{
			/* traverse tree and insert character into correct spot*/
			if (encoding[i] == '0')
			{
				/* intermediate node does not exist yet*/
				if (temp->left == NULL)
				{
					HuffNode * node = new HuffNode(item.second, 1);
					temp->left = node;
					temp = temp->left;
				}
				else
				{
					temp = temp->left;
				}
			}
			else
			{
				if (temp->right == NULL)
				{
					HuffNode * node = new HuffNode(item.second, 1);
					temp->right = node;
					temp = temp->right;
				}
				else
				{
					temp = temp->right;
				}
			}
		}
		temp = root;
	}
}

int main(int argc, char ** argv)
{
	vector<Message> codes;
	unordered_map<string, char> encodings;
	HuffNode * root = new HuffNode('t', 1);

	SOCKET ConnectSocket = INVALID_SOCKET;
	ConnectSocket = createSocket();
	
	cout << "building Huffman Tree" << endl;
	/* get and build Huffman Tree */
	receiveTree(ConnectSocket, encodings);
	constructTree(encodings, root);
	cout << "Huffman Tree built" << endl;

	/* signal that the tree has been built*/
	sendReady(ConnectSocket);

	cout << "decoding file" << endl;
	/* receive encoded file and decode*/
	receiveCodes(ConnectSocket, codes);
	decode(root, codes);
	cout << "file successfully decoded" << endl;

	shut(ConnectSocket);

	return 0;
}



