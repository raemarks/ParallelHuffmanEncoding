#include <fstream>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mpi.h>
#include <stdint.h>

#include "HuffmanEncoder.h"
#include "HuffmanTree.h"
#include "CompressedFile.h"
#include "Constants.h"

int p;
int mpirank;

#if 0
static vector<char> readFile(string input_file_name, uint64_t toRead,
		uint64_t offset)
{
	vector<char> result;
	FILE *f = fopen(input_file_name.c_str(), "r");
	int n;
	uint64_t total = 0;
	char c;

	if (f == NULL) {
		printf("Couldn't open file for reading\n");
		exit(1);
	}

	p = 1;
	ndivisions = 1;
	offsets = (uint64_t*) malloc(ndivisions*sizeof(uint64_t));
	offsets[0] = 0;


	while ((n = fread(&c, 1, 1, f)) != 0 && total < toRead) {
		total += n;
		result.push_back(c);
	}

	total = 0;

	return result;
}
#endif

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
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&mpirank);
	MPI_Comm_size(MPI_COMM_WORLD,&p);

	int divisions = 42;

	if (argc == 2)
	{
		if (string(argv[1]) == "test")
		{
			MPI_Finalize();
			return 0;
		}
		outputUsage();
		MPI_Finalize();
		return 0;
	}
	else if(argc == 3) {
		if (string(argv[1]) == "compress") {
			string to_compress = string(argv[2]);
			cout << "Compressing file " << to_compress << "..." << endl;

			struct stat s;
			if (stat(to_compress.c_str(),&s) < 0) {
				printf("Couldn't stat file, exiting.\n");
				exit(1);
			}

			HuffmanEncoder::CompressFileWithPadding(divisions, to_compress);

		}
		else if (string(argv[1]) == "decompress") {
			cout << "Decompressing " << argv[2] << "..." << endl;

			string to_decompress = argv[2];

			//separate extension from name in to_decompress
			if (to_decompress.size() < 5 ||
					to_decompress.substr(to_decompress.size()-4,4) != ".hez") {

				printf("Extension mismatch, nothing to do\n");
				MPI_Finalize();
				return (1);
			}

			HuffmanEncoder::DecompressFileWithPadding(to_decompress);
		}
		else {
			outputUsage();
			MPI_Finalize();
			return 0;
		}
	}
	else
	{
		outputUsage();
		MPI_Finalize();
		return 0;
	}

	MPI_Finalize();
	return 0;
}

