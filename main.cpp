#include <fstream>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "HuffmanEncoder.h"
#include "HuffmanTree.h"
#include "BinaryFile.h"

static vector<string> readFile(string input_file_name)
{
	vector<string> result;
	int fd = open(input_file_name.c_str(), O_RDONLY);
	int n;
	int total = 0;
	char buf[4096];

	if (fd < 0) {
		printf("Couldn't open file for reading\n");
		exit(1);
	}

	while ((n = read(fd, buf, 4096)) != 0) {
		total += n;
		result.push_back(string(buf, n));
	}

	total = 0;
	for (auto it = result.begin(); it != result.end(); ++it) {
		total += it->size();
	}
	return result;
}

//outputs HuffmanEncoder exe usage
void outputUsage()
{
	cout << "Usage: <exe name> <Action> <File1> <File2>" << endl;
	cout << "Actions (must be lower case):" << endl;
	cout << "test - runs buit-in unit test functions" << endl;
	cout << "compress - compresses <File1>" << endl;
	cout << "decompress - decompresses <File1> into <File2>" << endl;
}

int main(int argc, char* argv[])
{
	//calling for unit tests?
	if (argc == 2)
	{
		if (string(argv[1]) == "test")
		{
			return 0;
		}
		outputUsage();
		return 0;
	}
	else if(argc == 3) {
		if (string(argv[1]) == "compress") {
			string to_compress = string(argv[2]);
			cout << "Compressing file " << to_compress << "..." << endl;
			vector<string> file_contents = readFile(to_compress);

			HuffmanTree *coding_tree = HuffmanEncoder::huffmanTreeFromText(file_contents);

			vector<string> encoder = HuffmanEncoder::huffmanEncodingMapFromTree(coding_tree);

			vector<bool> raw_stream = HuffmanEncoder::toBinary(file_contents, encoder);

			string output_file_name = string(to_compress) + ".hez";
			BinaryFile::WriteToFile(raw_stream, output_file_name, encoder);

			delete coding_tree;
		}
		else if (string(argv[1]) == "decompress") {
			cout << "Decompressing " << argv[2] << "..." << endl;

			string to_decompress = argv[2];

			//separate extension from name in to_decompress
			if (to_decompress.size() < 5 ||
					to_decompress.substr(to_decompress.size()-4,4) != ".hez") {

					printf("Extension mismatch, nothing to do\n");
					return (1);
			}

			vector<string>* encoder;
			/* Get bits and map back from file */
			vector<bool> bits_from_file = BinaryFile::ReadFromFile(to_decompress, &encoder);
			/* Convert file bits back into text */
			string text = HuffmanEncoder::decodeBits(bits_from_file, *encoder);

			/* rite decompressed back to file */
			string outputFileName = string(argv[2]);
			/* Delete .hez extension */
			outputFileName = outputFileName.substr(0, outputFileName.size()-4);
			ofstream output_file(outputFileName);
			output_file << text;
			output_file.close();
		}
		else {
			outputUsage();
			return 0;
		}
	}
	else
	{
		outputUsage();
		return 0;
	}
}
