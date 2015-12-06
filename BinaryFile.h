#ifndef BINARY_FILE_H
#define BINARY_FILE_H

#include <fstream>
#include <vector>
#include <string>

using namespace std;
class BinaryFile
{
public:

	//writes a vector of strings to a file.  Assumes each character in each string is either a 0 or 1
	static void WriteToFile(const vector<bool> &content,
			const string &output_file_name, const vector<string>& huffmanMap);

	//reads a file written with writeToFile into a vector of bools
	static vector<bool> ReadFromFile(const string &input_file_name,
			vector<string>** huffmanMap);
};

#endif
