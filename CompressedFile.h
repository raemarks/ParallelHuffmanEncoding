#ifndef BINARY_FILE_H
#define BINARY_FILE_H

#include <fstream>
#include <vector>
#include <string>

using namespace std;
class CompressedFile
{
public:

	//writes a vector of strings to a file.  Assumes each character in each string is either a 0 or 1
	static void WriteToFile(const vector<bool> &content, FILE *output_file,
			const vector<string>& huffmanMap, uint64_t offset, uint64_t length);

	//reads a file written with writeToFile into a vector of bools
	static vector<bool> ReadFromFile(FILE *input_file, vector<string>** huffmanMap,
			uint64_t offset, uint64_t length);
};

#endif
