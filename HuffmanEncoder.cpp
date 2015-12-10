#include <string.h>
#include <math.h>
#include <cstdio>
#include <stdint.h>
#include <queue>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cerrno>

#include "CompressedFile.h"
#include "HuffmanEncoder.h"
#include "Constants.h"

struct HuffmanTreeCompare {
	bool operator()(HuffmanTree *a, HuffmanTree *b){
		return a->GetWeight() > b->GetWeight();
	}
};

static vector<char> readFile(string input_file_name, uint64_t toRead,
		uint64_t offset)
{
	vector<char> result;
	FILE *f = fopen(input_file_name.c_str(), "r");
	int n;
	uint64_t total = 0;
	char c;

	fseek(f, offset, 0);

	if (f == NULL) {
		printf("Couldn't open file for reading\n");
		exit(1);
	}

	while ((n = fread(&c, 1, 1, f)) != 0 && total < toRead) {
		total += n;
		result.push_back(c);
	}

	total = 0;

	return result;
}

void printChar(char c)
{
	printf("%d", (int) c);
}

	static uint64_t
calculateMetaDataSize(const vector<string>& huffmanMap)
{
	int size;
	int len;
	int length = stoi(huffmanMap[256]);

	if (length <= 8) {
		size = 512*sizeof(uint8_t);
		len = 8;
	}
	else if (length <= 16) {
		size = 512*sizeof(uint16_t);
		len = 16;
	}
	else if (length <= 32) {
		size = 512*sizeof(uint32_t);
		len = 32;
	}
	else if (length <= 64) {
		size = 512*sizeof(uint64_t);
		len = 64;
	}
	else {
		printf("Length not correct in calculate size. Length = %d\n", length);
		exit(1);
	}

	return size + sizeof(ndivisions) + ndivisions*sizeof(uint64_t) + sizeof(len);
}

/* Decompress the file with the number of divisions indicated by file.
 * Bytes are oadded out to a full byte with zeros at the end of each chunk
 * in the case that the compressed chunk has a bit count that is not
 * divisible by 8. */
	int
HuffmanEncoder::DecompressFileWithPadding(const string& filename)
{
	struct stat s;
	int fd = open(filename.c_str(), O_RDONLY);
	if (fstat(fd,&s) < 0) {
		printf("Couldn't stat file, exiting.\n");
		exit(1);
	}
	close(fd);

	FILE *inputFile = fopen(filename.c_str(), "r");
	vector<string>* huffmanMap;

	CompressedFile::ReadMetadataFromFile(inputFile, &huffmanMap);

	printf("%d: my offset: %ld\n", mpirank, offsets[mpirank]);
	vector<bool> compressedData = CompressedFile::ReadFromFile(inputFile,
			offsets[mpirank]);
	fclose(inputFile);

	printf("%d: Decoding %ld bits\n", mpirank, compressedData.size());
	string text = HuffmanEncoder::decodeBits(compressedData, *huffmanMap);

	/* Length in bytes of uncompressed data */
	uint64_t lengthToWrite = text.size();
	uint64_t offset = 0;

	/* Delete .hez extension */
	string output_filename = filename.substr(0, filename.size()-4);

	if (mpirank == 0) {
		int fd = creat(output_filename.c_str(), 0664);
		if (fd < 0) {
			printf("%d: Couldn't create output file %s\n", mpirank, output_filename.c_str());
			exit(1);
		}
		close(fd);
	}

	/* Parallel prefix to determine where to start writing uncompressed data in
	 * the file */
	int err = MPI_Scan(&lengthToWrite, &offset, 1, MPI_UINT64_T, MPI_SUM,
			MPI_COMM_WORLD);
	if (err) {
		printf("MPI_Scan failed!\n");
		return err;
	}

	if (mpirank == p-1) {
		/* Last rank knows how big the final file is, so it will take care of
		 * allocating it. Here, offset is the total length of the uncompressed
		 * file. */
		err = truncate(output_filename.c_str(), offset);
		if (err) {
			printf("Issue with truncate. Error: %s\n", strerror(errno));
			exit (1);
		}
	}
	/* Make sure file is truncated first. */
	MPI_Barrier(MPI_COMM_WORLD);
	FILE *outputFile = fopen(output_filename.c_str(), "r+");
	printf("Decompressing %s to %s\n", filename.c_str(), output_filename.c_str());
	/* Adjust so we write at the beginning of the chunk, not the end */
	offset -= lengthToWrite;

	fseek(outputFile, offset, 0);
	fwrite(text.c_str(), 1, lengthToWrite, outputFile);
	fclose(outputFile);

	return 0;
}

/* Compress the file with the number of divisions indicated by the input
 * parameter. Pad out to a full byte with zeros at the end of each chunk to
 * compress in the case that the compressed chunk has a bit count that is not
 * divisible by 8. */
	int
HuffmanEncoder::CompressFileWithPadding(int divisions, const string& filename)
{
	/* TODO:
	 * Initially, do a check to see if p = ndivisions. At first, just pass in p.
	 * more on that later for experimentation */
	/* Stat the file. */
	struct stat s;
	int fd = open(filename.c_str(), O_RDONLY);
	if (fstat(fd,&s) < 0) {
		printf("Couldn't stat file, exiting.\n");
		exit(1);
	}
	close(fd);

	uint64_t fileLength = (uint64_t)s.st_size;
	printf("%s %ld %ld\n", filename.c_str(), s.st_ino, s.st_size);
	printf("File length: %ld\n", s.st_size);
	ndivisions = (uint32_t)p;
	offsets = (uint64_t*) malloc (sizeof(uint64_t)*p);
	chunkLengths = (uint64_t*) malloc (sizeof(uint64_t)*p);

	if (mpirank == 0) printf("%d: Setting offsets\n", mpirank);
	for (int i = 0; i < p; i++) {
		offsets[i] = i*(fileLength/p);
		if (i == mpirank)
			printf("My rank: %d, my offset: %ld\n", mpirank, offsets[i]);
	}
	for (int i = 0; i < p; i++) {
		if (i == p-1)
			chunkLengths[i] = ceil(fileLength/p);
		else
			chunkLengths[i] = fileLength/p;
		if (i == mpirank)
			printf("My rank: %d, my chunkLength: %ld\n", mpirank, chunkLengths[i]);
	}

	printf("%d: Reading file at offset %ld with chunk size %ld\n", mpirank,
			offsets[mpirank], chunkLengths[mpirank]);
	vector<char> text = readFile(filename, chunkLengths[mpirank], offsets[mpirank]);
	printf("%d: read file! Text length: %ld\n", mpirank, text.size());

	HuffmanTree *coding_tree = HuffmanEncoder::huffmanTreeFromText(text);

	vector<string> encoder = HuffmanEncoder::huffmanEncodingMapFromTree(coding_tree);

	vector<bool> raw_stream = HuffmanEncoder::toBinary(text, encoder);

	string output_file_name = filename + ".hez";

	/* Length in bytes of compressed data */
	uint64_t lengthToWrite = (uint64_t)ceil((double)raw_stream.size()/8);
	printf("%d: My length to write: %ld, raw size: %ld\n", mpirank, lengthToWrite, raw_stream.size());
	/* Leave room to print length of chunk */
	lengthToWrite += sizeof(size_t);
	uint64_t offset = 0;
	metadataOffset = calculateMetaDataSize(encoder);
	if (mpirank == 0) {
		printf("metadata offset: %d\n", metadataOffset);
	}

	/* Parallel prefix to determine where to start writing uncompressed data in
	 * the file */
	if (mpirank == 0) {
		int fd = creat(output_file_name.c_str(), 0664);
		if (fd < 0) {
			printf("%d: Couldn't create output file %s\n", mpirank, output_file_name.c_str());
			exit(1);
		}
		close(fd);
	}


	printf("%d: Calling MPI scan...\n", mpirank);
	int err = MPI_Scan(&lengthToWrite, &offset, 1, MPI_UINT64_T, MPI_SUM,
			MPI_COMM_WORLD);
	if (err) {
		printf("MPI_Scan failed!\n");
		return err;
	}
	printf("%d: Finished MPI scan...\n", mpirank);

	if (mpirank == p-1) {
		/* Last rank knows how big the final file is, so it will take care of
		 * allocating it. Here, offset is the total length of the compressed
		 * file. */
		printf("Truncating...\n");
		err = truncate(output_file_name.c_str(), offset + metadataOffset);
		printf("Truncated...\n");
		/* TODO: Don't forget to allocate space for the metadata!!! */
		if (err) {
			printf("Issue with truncate. Error: %s\n", strerror(errno));
			return (1);
		}
	}
	/* Make sure file is truncated first. */
	MPI_Barrier(MPI_COMM_WORLD);

	FILE *output_file = fopen(output_file_name.c_str(), "r+");
	printf("Compressing %s to %s\n", filename.c_str(), output_file_name.c_str());
	if(output_file == NULL) {
		printf("%d: Couldn't open output_file %s!!!\n", mpirank, output_file_name.c_str());
		exit(1);
	}

	/* Adjust so we write at the beginning of the chunk, not the end */
	offset -= lengthToWrite;
	/* Adjust for metadata so we get absolute offset from beginning of file */
	offset += metadataOffset;

	/* Gather offsets from all files for metadata writing */
	printf("%d: Gathering offsets...my offset: %ld\n", mpirank, offset);
	err = MPI_Gather(&offset, 1, MPI_UINT64_T, offsets, 1, MPI_UINT64_T, 0,
			MPI_COMM_WORLD);
	printf("%d: Gathered offsets...my offset: %ld\n", mpirank, offsets[mpirank]);

	if (mpirank == 0) {
		printf("%d: Writing metadata!\n", mpirank);
		CompressedFile::WriteMetadataToFile(output_file, encoder);
	}
	printf("%d: my new offset: %ld\n", mpirank, offsets[mpirank]);

	printf("%d: Writing my chunk to the file!\n", mpirank);
	CompressedFile::WriteToFile(output_file, raw_stream, offset);
	printf("%d: Wrote my chunk to the file!\n", mpirank);
	fclose(output_file);

	return 0;
}

/* Compress the file with the number of divisions indicated by the input
 * parameter. Do not pad out to a full byte at the end of each chunk to
 * compress in the case that the compressed chunk has a bit count that is not
 * divisible by 8. */
	int
HuffmanEncoder::CompressFileWithoutPadding(int divisions, const string& filename)
{
	return 0;
}

	HuffmanTree*
HuffmanEncoder::huffmanTreeFromText(vector<char> data)
{
	//Builds a Huffman Tree from the supplied vector of strings.
	//This function implement's Huffman's Algorithm as specified in page
	//456 of the book.
	uint64_t myFreqMap[256], globalFreqMap[256];
	memset(myFreqMap, 0, 256);

	//filename = "tft" + std::to_string((long long int)mpirank);
	//FILE *outfile = fopen("tft", "w");


	unordered_map<char, uint64_t> freqMap{};
	for (auto it = data.begin(); it != data.end(); ++it) {
		freqMap[*it]++;
	}

	for (auto it = freqMap.begin(); it != freqMap.end(); ++it) {
		myFreqMap[(int)(unsigned char)it->first] = it->second;
	}

	/* Get global frequencies so all nodes have same Huffman coding dictionaries
	*/
	int err = MPI_Allreduce(myFreqMap, globalFreqMap, 256, MPI_UINT64_T, MPI_SUM,
			MPI_COMM_WORLD);
	if (err) {
		printf("Problem with MPI all reduce. Exiting...\n");
		exit(1);
	}

	priority_queue<HuffmanTree*,vector<HuffmanTree*>,HuffmanTreeCompare> forest{};

	for (int i = 0; i < 256; i++) {
		if (globalFreqMap[i] == 0) {
			continue;
		}
		HuffmanTree *tree = new HuffmanTree((char)i, globalFreqMap[i]);
		if (tree->GetWeight() == 0) {
			printf("Weight is 0!!!********************************************************8\n");
		}
		forest.push(tree);
	}

	printf("%ld trees in forest, data size: %ld\n", forest.size(), data.size());

	while (forest.size() > 1) {
		HuffmanTree *smallest = forest.top();
		forest.pop();
		HuffmanTree *secondSmallest = forest.top();
		forest.pop();
		HuffmanTree *tree = new HuffmanTree(smallest, secondSmallest);
		forest.push(tree);
	}
	forest.top()->Print();

	return forest.top();
}

	HuffmanTree*
HuffmanEncoder::huffmanTreeFromMap(vector<string> huffmanMap)
{
	//Generates a Huffman Tree based on the supplied Huffman Map.Recall that a
	//Huffman Map contains a series of codes(e.g. 'a' = > 001).Each digit(0, 1)
	//in a given code corresponds to a left branch for 0 and right branch for 1.

	HuffmanTree *tree = new HuffmanTree(new HuffmanInternalNode(NULL, NULL));

	for (int i = 0; i < 256; i++) {
		char c = (char) i;
		string code = huffmanMap[i];
		if (code.length() == 0)
			continue;
		HuffmanInternalNode *n = (HuffmanInternalNode*)tree->GetRoot();

		for (unsigned int j = 0; j < code.length() -1; ++j) {
			if (code[j] == '0') {
				if (n->GetLeftChild() != NULL) {
					n = (HuffmanInternalNode*) n->GetLeftChild();
				}
				else {
					n->SetLeftChild(new HuffmanInternalNode(NULL, NULL));
					n = (HuffmanInternalNode*) n->GetLeftChild();
				}
			}
			else {
				if (n->GetRightChild() != NULL) {
					if (n->IsLeaf()) {
						//tree->Print();
						exit(1);
					}
					n = (HuffmanInternalNode*) n->GetRightChild();
				}
				else {
					n->SetRightChild(new HuffmanInternalNode(NULL, NULL));
					n = (HuffmanInternalNode*) n->GetRightChild();
				}
			}
		}
		c = code[code.length() -1];
		if (c == '0') {
			n->SetLeftChild(new HuffmanLeafNode((char)i, 0));
		}
		else {
			n->SetRightChild(new HuffmanLeafNode((char)i, 0));
		}
	}

	//printf("Tree created from map:\n");
	//tree->Print();

	return tree;
}

	void
recursiveBuildMapFromTree(HuffmanNode *n, string s, vector<string>* m,
		unsigned int *max)
{
	if (n->IsLeaf()) {
		//Base case
		HuffmanLeafNode *ln = (HuffmanLeafNode*) n;
		int value = (int) ((unsigned char)ln->GetValue());
		if (s.size() > *max)
			*max = s.size();
		(*m)[value] = s;
		return;
	}
	else {
		HuffmanInternalNode *in = (HuffmanInternalNode*) n;

		if (in->GetLeftChild() != NULL) {
			recursiveBuildMapFromTree(in->GetLeftChild(), s + "0", m, max);
		}
		if (in->GetRightChild() != NULL) {
			recursiveBuildMapFromTree(in->GetRightChild(), s + "1", m, max);
		}
		return;
	}
}

	vector<string>
HuffmanEncoder::huffmanEncodingMapFromTree(HuffmanTree *tree)
{
	//Generates a Huffman Map based on the supplied Huffman Tree.  Again, recall
	//that a Huffman Map contains a series of codes(e.g. 'a' = > 001).Each digit(0, 1)
	//in a given code corresponds to a left branch for 0 and right branch for 1.
	//As such, a given code represents a pre-order traversal of that bit of the
	//tree.  I used recursion to solve this problem.
	vector<string> huffmanMap(257);
	string s = "";
	unsigned int max = 0;

	recursiveBuildMapFromTree(tree->GetRoot(), s, &huffmanMap, &max);

	/* Store the length of the longest code at the end of the map */
	huffmanMap[256] = std::to_string((long long int)max);

	return huffmanMap;
}

	void
HuffmanEncoder::writeEncodingMapToFile(vector<string> huffmanMap, string file_name)
{
	/* The map will take up 2*alphabet size, so each character in the alphabet
	 * gets 2 bytes, the 1st counts the number of bits in the code, and the 2nd
	 * contains the actual bits. We are encoding at the byte level, so our
	 * alphabet is of size 256.*/

	ofstream outFile(file_name.c_str(), ofstream::binary);

	for (unsigned int i = 0; i < huffmanMap.size() - 2; i++) {
		outFile << i << " : " << huffmanMap[i] << endl;
	}
	outFile << "Length of codes: " << huffmanMap[256] << endl;

	/* Write #bits needed to store the max code length */
	outFile.close();
}

	vector<string>
HuffmanEncoder::readEncodingMapFromFile(string file_name)
{
	//ifstream inFile(file_name.c_str());
	ifstream inFile(file_name.c_str(), ifstream::binary);
	void *toRead;
	int length, len, size;
	uint64_t c;
	vector<string> huffmanMap(256);;

	if (!inFile.good()) {
		printf("Couldn't open map file for reading! \n");
		exit(1);
	}
	inFile.read((char*)(&len), (int)sizeof(uint32_t));

	switch(len) {
		case 8:
			size = 512*sizeof(uint8_t);
			break;
		case 16:
			size = 512*sizeof(uint16_t);
			break;
		case 32:
			size = 512*sizeof(uint32_t);
			break;
		case 64:
			size = 512*sizeof(uint64_t);
			break;
		default:
			printf("len is wrong: %d\n", len);
			exit(1);
	}
	toRead = malloc(size);

	inFile.read((char*)toRead, size);
	inFile.close();
	for (unsigned int i = 0; i < 512; i+=2) {
		switch(len) {
			case 8:
				length = (int) ((uint8_t*)toRead)[i];
				c = (uint64_t) ((uint8_t*)toRead)[i+1];
				break;
			case 16:
				length = (int) ((uint16_t*)toRead)[i];
				c = (uint64_t) ((uint16_t*)toRead)[i+1];
				break;
			case 32:
				length = (int) ((uint32_t*)toRead)[i];
				c = (uint64_t) ((uint32_t*)toRead)[i+1];
				break;
			case 64:
				length = (int) ((uint64_t*)toRead)[i];
				c = (uint64_t) ((uint64_t*)toRead)[i+1];
				break;
		}

		/* Decode bits into chars */
		string s = "";
		for (int j = 0; j < length; ++j) {
			if (c & 1)
				s = "1" + s;
			else
				s = "0" + s;
			c >>= 1;
		}
		/* Every char in alphabet gets 2 bytes, so the code we are reading
		 * belongs to the char value of index/2 */
		huffmanMap[i/2] = s;
	}

	return huffmanMap;
}

	string
HuffmanEncoder::decodeBits(vector<bool> bits, vector<string> huffmanMap)
{
	HuffmanTree *tree = HuffmanEncoder::huffmanTreeFromMap(huffmanMap);

	vector<string> map = HuffmanEncoder::huffmanEncodingMapFromTree(tree);
	HuffmanEncoder::writeEncodingMapToFile(map, "encmap.map");

	HuffmanNode *n = tree->GetRoot();
	ostringstream result{};
	uint64_t last_index = 0;
	string stack;
	int num_decoded = 0;

	//tree->Print();

	for (auto it = bits.begin(); it != bits.end(); ++it) {
		bool bit = *it;
		if (bit == false) {
			stack += "0";
			n = ((HuffmanInternalNode*)n)->GetLeftChild();
		}
		else {
			stack += "1";
			n = ((HuffmanInternalNode*)n)->GetRightChild();
		}
		if (n->IsLeaf()) {
			result << ((HuffmanLeafNode*)n)->GetValue();
			num_decoded++;
			n = tree->GetRoot();
			stack = "";
		}
		last_index++;
	}

	/* TODO: perhaps the priority queue is different for each? That might
	 * explain it. Compare frequencies, that can't be wrong. Issue is we have
	 * different huffman maps on each run. Although, that might not be a problem
	 * on write. The files are slightly different, make sure it's writing to a
	 * good offset. Maybe try writing/reading garbage from that spot or
	 * something, print out the first few chars, idk. Print where the offsets
	 * and such are. Figure out exactly what is going where and if the way it's
	 * getting compressed/decompressed differently is a problem. */


	return result.str();
}

	vector<bool>
HuffmanEncoder::toBinary(vector<char> text, vector<string> huffmanMap)
{
	vector<bool> result;
	uint64_t num_encoded = 0, bits = 0;

	string filename = "test" + std::to_string((long long int)mpirank);
	FILE *f = fopen(filename.c_str(), "w");
	for (unsigned int i = 0; i < huffmanMap.size(); i++) {
		fprintf(f, "%d: %s\n", i, huffmanMap[i].c_str());
	}
	for (auto it = text.begin(); it != text.end(); ++it) {
		char c = *it;
		num_encoded++;

		string code = huffmanMap[(int)((unsigned char)c)];

		for (auto it2 = code.begin(); it2 != code.end(); ++it2) {
			char b = *it2;
			bits++;
			result.push_back(b == '1');
		}
	}

	printf("%d: encoded %ld, pushed %ld bits, size: %ld\n",mpirank, num_encoded, bits, result.size());


	return result;
}
