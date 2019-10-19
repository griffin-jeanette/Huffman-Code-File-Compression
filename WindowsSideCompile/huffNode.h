#pragma once
/*
	class containing tree nodes
*/

#ifndef HUFFNODE_HPP
#define HUFFNODE_HPP

class HuffNode
{
private:
	int freq;
	char c;

public:
	HuffNode * left{ nullptr };
	HuffNode * right{ nullptr };

	// constructor
	HuffNode(const char ch, const int num);

	int getFreq();
	char getChar();
};

#endif