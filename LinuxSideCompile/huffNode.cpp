/*
class containing tree nodes
*/

#include <iostream>
#include "huffNode.h"


// constructor
HuffNode::HuffNode(const char ch, const int num)
{
  c = ch;
  freq = num;
}

int HuffNode::getFreq()
{
  return freq;
}

char HuffNode::getChar()
{
  return c;
}
