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

using namespace std;

struct CompressedData {
	uint64_t byteOffset;
	uint64_t bitOffset;
	uint64_t nbytes;
	uint64_t nbits;
	void *data;
};

class HuffmanEncoder
{
	public:
		static HuffmanTree* huffmanTreeFromFrequencyMap(uint64_t *FreqMap);
		static void frequencyMapFromText(vector<char> data, uint64_t *FreqMap);

	static int DecompressFileWithPadding(const string& filename);
	/* Compress the file with the number of divisions indicated by the input
	 * parameter. Pad out to a full byte with zeros at the end of each chunk to
	 * compress in the case that the compressed chunk has a bit count that is not
	 * divisible by 8. */
	static int CompressFileWithPadding(int divisions, const string& filename);

	/* Compress the file with the number of divisions indicated by the input
	 * parameter. Do not pad out to a full byte at the end of each chunk to
	 * compress in the case that the compressed chunk has a bit count that is not
	 * divisible by 8. */
	static int CompressFileWithoutPadding(int divisions, const string& filename);
	static void CalculateOffsets();

	/* Finds the smallest tree in a given forest, allowing for
	 * a single skip */
	static int
		findSmallestTree(vector<HuffmanTree *>& forest, int index_to_ignore = -1);

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
		toBinary(vector<char> text, vector<string> huffmanMap);
};

#endif

