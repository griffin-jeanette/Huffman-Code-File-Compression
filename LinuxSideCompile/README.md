## Introduction

This program uses Huffman Trees to compress a file and then send it over a TCP socket from a Linux machine to a Windows machine and then decode the file.

### Compile Directions:
This program was compiled using Microsoft Visual Studio 2015 to compile **ConsileApplication2.cpp, huffNode.h, huffNode.cpp, wSender.h** and **wSender.cpp**. On Linux, g++ version 7.4.0 was used as the following:
    g++ -o findCodes findCodes.cpp huffNode.h huffNode.cpp sender.h sender.cpp

### Limitations:
cannot correctly send files with '%' or '$' symbols.  The '%' symbol instead inserts a '\n' during the decoding phase while the '$' symbol is completely ignored
