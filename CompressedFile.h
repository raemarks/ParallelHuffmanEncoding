#ifndef COMPRESSED_FILE_H
#define COMPRESSED_FILE_H

#include <fstream>
#include <vector>
#include <string>

using namespace std;
class CompressedFile
{
public:

	static void WriteMetadataToFile(FILE *outputFile,
			const vector<string>& huffmanMap);

	/* Writes a vector of chars to a file. Assumes each character in each string
	 * is either a 0 or 1 */
	static void WriteToFile(FILE *outputFile, const vector<bool> &content,
			uint64_t offset);

	static void ReadMetadataFromFile(FILE *inputFile, vector<string>** huffmanMap);

	/* Reads a chunk of a compressed file written with writeToFile into a
	 * vector of bools */
	static vector<bool> ReadFromFile(FILE *inputFile,	uint64_t offset);
};

#endif
