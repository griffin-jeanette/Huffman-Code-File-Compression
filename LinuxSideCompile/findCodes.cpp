/*
This program reads through a file an creates finds the corresponding
Huffman codes for that file
*/

#include <fstream>
#include <string>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <unistd.h>
#include <stdint.h>
#include "huffNode.h"
#include "sender.h"
using namespace std;

/* custom compare struct used in priority queue */
struct Compare
{
  bool operator() (HuffNode * node1, HuffNode * node2)
  {
    return node1->getFreq() > node2->getFreq();
  }
};

/* reads through a file and keeps track of the frequency of each character */
void countChars(string filename, unordered_map<char, int> & counts)
{
  ifstream myFile(filename);
  char c;

  while (!myFile.eof())
  {
    myFile.get(c);

    auto it = counts.find(c);

    /* char is already in map */
    if (it != counts.end())
      it->second++;
    else
    {
      if (c != '\n')
        counts[c] = 1;
      else
      {
        /* special case where we store the '\n'
           character as the '%' character       */
        it = counts.find('%');
        if (it != counts.end())
          it->second++;
        else
          counts['%'] = 1;
      }
    }
  }
}

/* function to generate huffman encodings */
void generateCodes(HuffNode * root, string str,
                    unordered_map<string, char> & codes,
                    unordered_map<char, string> & charCodes)
{
  if (!root)
    return;

  if (root->getChar() != '$')
  {
    codes[str] = root->getChar();
    charCodes[root->getChar()] = str;
  }

  generateCodes(root->left, str + "0", codes, charCodes);
  generateCodes(root->right, str + "1", codes, charCodes);
}

/* constructs a huffman tree based on the frequency of characters
   occurring in a file                                            */
void constructTree(unordered_map<char, int> & counts,
                   unordered_map<string, char> & codes,
                   unordered_map<char, string> & charCodes,
                   priority_queue<HuffNode*, vector<HuffNode*>, Compare> & pq)
{
  /* put element of counts map into min priority queue */
  for (auto item: counts)
  {
    HuffNode * node = new HuffNode(item.first, item.second);
    pq.push(node);
  }

  /* start constructing Huffman tree */
  while (pq.size() != 1)
  {
    HuffNode * left = pq.top();
    pq.pop();

    HuffNode * right = pq.top();
    pq.pop();

    /* insert a '$' as a placeholder */
    HuffNode * top = new HuffNode('$', left->getFreq() + right->getFreq());

    top->left = left;
    top->right = right;

    pq.push(top);
  }

  generateCodes(pq.top(), "", codes, charCodes);
}

/* function to compress character encodings into a bits */
void shiftBytes(unordered_map<char, string> & codes, string filename,
                HuffNode * root, vector<Message> & messages)
{
  ifstream myFile(filename);
  Message message;
  char c;
  uint32_t bytes = 0;
  uint32_t b = 0;
  int count = 0;
  string encoding = "";
  string code = "";
  int index = 0;

  while (!myFile.eof())
  {
    myFile.get(c);

    if (c != '\n')
    {
      encoding = codes[c];
      code += encoding;

      /* convert encoding of zeros and ones to a representation
         in binary                                              */
      for (int i = 0; i < encoding.length(); i++)
      {
        if (encoding[i] == '0')
        {
          if (count > 0)
            b <<= 1;
          b |= 0;
          count += 1;
        }
        else if (encoding[i] == '1')
        {
          if (count > 0)
            b <<= 1;
          b |= 1;
          count += 1;
        }
        /* all 32 bits of uint32_t are filled */
        if (count == 32)
        {
          /* put into message struct */
          Message message;
          message.bytes = count;
          message.code = b;
          messages.push_back(message);

          /* reset */
          b = 0;
          count = 0;
        }
      }
    }
    else
    {
      encoding = codes['%'];
      code += encoding;

      /* convert encoding of zeros and ones to a representation
         in binary                                              */
      for (int i = 0; i < encoding.length(); i++)
      {
        if (encoding[i] == '0')
        {
          if (count > 0)
            b <<= 1;
          b |= 0;
          count += 1;
        }
        else if (encoding[i] == '1')
        {
          if (count > 0)
            b <<= 1;
          b |= 1;
          count += 1;
        }
        /* all 32 bits of uint32_t are filled */
        if (count == 32)
        {
          /* put into message struct */
          Message message;
          message.bytes = count;
          message.code = b;
          messages.push_back(message);

          /* reset */
          b = 0;
          count = 0;
        }
      }
    }
  }

  /* store off any code that is not a full 32 bits */
  message.bytes = count;
  message.code = b;
  messages.push_back(message);
}

/* send encoded file to other side of socket */
void sendFile(int socketfd, vector<Message> & messages)
{
  for (int i = 0; i < messages.size(); i++)
    sendData(socketfd, messages[i]);
}

/* sends Huffman Tree header containing tree encoding */
void encodeFile(unordered_map<string, char> & huffCodes, string & codeStr)
{
  string part1 = "";
  string part2 = "";
  codeStr += "[";

  /* create code string of form:
    [(char1):(101010),(char2):(encoding2),] */
  for (auto code: huffCodes)
  {
    part1 = code.first;
    part2 = code.second;
    codeStr +=  part2 + ":" + part1 + ",";
  }
  codeStr += "]";
}

int main()
{
  string filename = "test.txt";
  string codeStr = "";
  unordered_map<char, int> counts;
  unordered_map<string, char> codes;
  unordered_map<char, string> charCodes;
  HuffNode * root;
  priority_queue<HuffNode*, vector<HuffNode*>, Compare> pq;
  vector<Message> messages;

  cout << endl;
  cout << "beginning to transfer file" << endl;

  /* calculate frequency of characters in file */
  countChars(filename, counts);

  /* create Huffman tree based on character frequencies */
  constructTree(counts, codes, charCodes, pq);

  /* use Huffman tree to encode file */
  encodeFile(codes, codeStr);

  /* take file encodings and store them as binary */
  shiftBytes(charCodes, filename, pq.top(), messages);

  int socketfd = createSocket();

  /* send Huffman tree header information */
  sendDataStr(socketfd, codeStr);

  /* wait for ready and then send file encoding */
  receiveData(socketfd);
  sendFile(socketfd, messages);

  cout << "file transfer complete" << endl << endl;

  close(socketfd);

  return 0;
}
