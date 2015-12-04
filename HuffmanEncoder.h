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
		/* Finds the smallest tree in a given forest, allowing for
		 * a single skip */
	static int
		findSmallestTree(vector<HuffmanTree *>& forest, int index_to_ignore = -1);

	static int CompressFile(string source, string dest);
	static int DecompressFile(string source, string dest);

	/* Generates a Huffman character tree from the supplied
	 * text */
	static HuffmanTree *huffmanTreeFromText(vector<string> data);

	/* Generates a Huffman character tree from the supplied
	 * encoding map */
	// NOTE: I used a recursive helper function to solve this!
	static HuffmanTree *huffmanTreeFromMap(vector<string> huffmanMap);

	/* Generates a Huffman encoding map from the supplied
	 * Huffman tree */
	//NOTE: I used a recursive helper function to solve this!
	static vector<string> huffmanEncodingMapFromTree(HuffmanTree *tree);

	/* Writes an encoding map to file.  Needed for
	 * decompression. */
	static void	writeEncodingMapToFile(vector<string> huffmanMap, string file_name);

	/* Reads an encoding map from a file.  Needed for
	 * decompression. */
	static vector<string> readEncodingMapFromFile(string file_name);

	/* Converts a vector of bits (bool) back into readable text
	 * using the supplied Huffman map */
	static string decodeBits(vector<bool> bits, vector<string> huffmanMap);

	/* Using the supplied Huffman map compression, converts the
	 * supplied text into a series of bits (boolean values) */
	static vector<bool>
		toBinary(vector<string> text, vector<string> huffmanMap);
};

#endif

