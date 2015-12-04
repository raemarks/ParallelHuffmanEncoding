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
	char buf[4096];

	if (fd < 0) {
		printf("Couldn't open file for reading\n");
		exit(1);
	}

	while ((n = read(fd, buf, 4096)) != 0) {
		result.push_back((string(buf)).substr(0, n));
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
	else if(argc == 3)
	{
		if (string(argv[1]) == "compress")
		{
			string to_compress = string(argv[2]);
			cout << "Compressing file " << to_compress << "..." << endl;
			vector<string> file_contents = readFile(to_compress);

			//PA #2: build tree
			HuffmanTree *coding_tree = HuffmanEncoder::huffmanTreeFromText(file_contents);

			//PA #2: generate encoding map
			vector<string> encoder = HuffmanEncoder::huffmanEncodingMapFromTree(coding_tree);

			//PROVIDED: convert file into vector of bits
			vector<bool> raw_stream = HuffmanEncoder::toBinary(file_contents, encoder);

			//PROVIDED: write vector of bits to separate file
			vector<string> pieces = StringSplitter::split(string(to_compress), ".");
			string file_name = pieces[0];
			string extension = "";
			if (pieces.size() > 1)
			{
				extension = pieces[1];
			}
			string output_file_name = string(file_name) + ".pa2c";
			BinaryFile::WriteToFile(raw_stream, output_file_name);

			//PA #2: write map to file
			string map_file = string(file_name) + ".pa2m";
			HuffmanEncoder::writeEncodingMapToFile(encoder, map_file);

			delete coding_tree;
		}
		else
		{
			outputUsage();
			return 0;
		}
	}
	else if (argc == 4)
	{
		if (string(argv[1]) == "decompress")
		{
			cout << "Decompressing " << argv[2] << "..." << endl;

			string to_decompress = argv[2];
			string extract_location = argv[3];

			//separate extension from name in to_decompress
			vector<string> pieces = StringSplitter::split(string(to_decompress), ".");
			string file_name = pieces[0];
			string extension = "";
			if (pieces.size() > 1)
			{
				extension = pieces[1];
			}

			//PA #2: get bits back from file
			vector<bool> bits_from_file = BinaryFile::ReadFromFile(to_decompress);

			//PA #2: read map from file
			vector<string> encoder_from_file = HuffmanEncoder::readEncodingMapFromFile(file_name + ".pa2m");

			//PA #2: convert file bits back into text
			string text = HuffmanEncoder::decodeBits(bits_from_file, encoder_from_file);

			//PA #2: write decompressed back to file
			ofstream output_file{ extract_location };
			output_file << text;
			output_file.close();
		}
		else
		{
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