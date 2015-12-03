#ifndef HUFFMAN_ENCODER_H
#define HUFFMAN_ENCODER_H

#include "HuffmanTree.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include "BinaryFile.h"
#include "StringSplitter.h"


using namespace std;

class HuffmanEncoder
{
public:
	/* PA #2 TODO: finds the smallest tree in a given forest, allowing for
	 * a single skip */
	static int
	    findSmallestTree(vector<HuffmanTree *>& forest,
		int index_to_ignore = -1);

	/* PA #2 TOOD: Generates a Huffman character tree from the supplied
	 * text */
	static HuffmanTree *
	    huffmanTreeFromText(vector<string> data);

	/* PA #2 TODO: Generates a Huffman character tree from the supplied
	 * encoding map */
	// NOTE: I used a recursive helper function to solve this!
	static HuffmanTree *
	    huffmanTreeFromMap(string* huffmanMap);

	/* PA #2 TODO: Generates a Huffman encoding map from the supplied
	 * Huffman tree */
	//NOTE: I used a recursive helper function to solve this!
	static string*
	    huffmanEncodingMapFromTree(HuffmanTree *tree);

	/* PA #2 TODO: Writes an encoding map to file.  Needed for
	 * decompression. */
	static void
	    writeEncodingMapToFile(string* huffmanMap,
		string file_name);

	/* PA #2 TODO: Reads an encoding map from a file.  Needed for
	 * decompression. */
	static string*
	    readEncodingMapFromFile(string file_name);

	/* PA #2 TODO: Converts a vector of bits (bool) back into readable text
	 * using the supplied Huffman map */
	static string
	    decodeBits(vector<bool> bits,
		string* huffmanMap);

	/* PA #2 TODO: Using the supplied Huffman map compression, converts the
	 * supplied text into a series of bits (boolean values) */
	static vector<bool>
	    toBinary(vector<string> text, string* huffmanMap);

};

#endif

