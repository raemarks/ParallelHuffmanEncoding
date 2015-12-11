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

uint32_t ndivisions;
/* Array storing the chunk offsets */
uint64_t *offsets;
/* Amount of the beginning of the compressed file reserved for the huffman map
 * and the division offsets. */
uint32_t metadataOffset;
/* Length of each chunk, with chunks divided up with the devisions */
uint64_t *chunkLengths;

vector<char> *chunks;

vector<bool> *compressedChunks;

std::string *decompressedChunks;

int *chunksPerProc;

uint64_t *newLengths;

uint64_t *newOffsets;

vector<int> myChunks = {};

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

	chunkLengths = new uint64_t[ndivisions];
	chunks = new vector<char>[ndivisions];
	compressedChunks = new vector<bool>[ndivisions];
	newLengths = new uint64_t[ndivisions];
	newOffsets = new uint64_t[ndivisions];
	decompressedChunks = new string[ndivisions];
	chunksPerProc = new int[p];
	memset(chunksPerProc, 0, sizeof(int)*p);

	int rank = 0;
	for (unsigned int i = 0; i < ndivisions; i++) {
		if (rank % p == mpirank) {
			myChunks.push_back(i);
		}
		chunksPerProc[rank]++;
		rank++;
	}

	/****************************************************************************************/
	for (auto it = myChunks.begin(); it != myChunks.end(); it++) {
		compressedChunks[*it] = CompressedFile::ReadFromFile(inputFile,
				offsets[*it]);
		printf("%d: Read chunk %d at offset %ld, decoding...\n", mpirank, *it,
				offsets[*it]);
		decompressedChunks[*it] = HuffmanEncoder::decodeBits(compressedChunks[*it], *huffmanMap);
		printf("%d: Finished decoding chunk %d\n", mpirank, *it);
		newLengths[*it] = decompressedChunks[*it].size();
	}
	fclose(inputFile);

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

	metadataOffset = 0;
	HuffmanEncoder::CalculateOffsets();

	if (mpirank == p-1) {
		/* Last rank knows how big the final file is, so it will take care of
		 * allocating it. Here, offset is the total length of the uncompressed
		 * file. */
		uint64_t uncompressedSize = 0;
		for (unsigned int i = 0; i < ndivisions; i++) {
			uncompressedSize += newLengths[i];
		}
		int err = truncate(output_filename.c_str(), uncompressedSize);
		if (err) {
			printf("Issue with truncate. Error: %s\n", strerror(errno));
			exit (1);
		}
	}
	/* Make sure file is truncated first. */
	MPI_Barrier(MPI_COMM_WORLD);
	FILE *outputFile = fopen(output_filename.c_str(), "r+");
	//printf("Decompressing %s to %s\n", filename.c_str(), output_filename.c_str());
	/* Adjust so we write at the beginning of the chunk, not the end */
	//printf("%d: My length to write: %ld, my offset: %ld\n", mpirank, lengthToWrite, offset);

	for (auto it = myChunks.begin(); it != myChunks.end(); it++) {
		fseek(outputFile, newOffsets[*it], 0);
		fwrite(decompressedChunks[*it].c_str(), 1, newLengths[*it], outputFile);
		printf("%d: Wrote chunk %d to the file at offset %ld!\n", mpirank,
				*it, newOffsets[*it]);
	}
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
	/* Stat the file. */

	struct stat s;
	int fd = open(filename.c_str(), O_RDONLY);
	if (fstat(fd,&s) < 0) {
		printf("Couldn't stat file, exiting.\n");
		exit(1);
	}
	close(fd);

	uint64_t fileLength = (uint64_t)s.st_size;
	if (mpirank == 0) printf("File length: %ld\n", fileLength);
	ndivisions = (uint32_t)divisions;
	offsets = new uint64_t[ndivisions];
	chunkLengths = new uint64_t[ndivisions];
	chunks = new vector<char>[ndivisions];
	compressedChunks = new vector<bool>[ndivisions];
	newLengths = new uint64_t[ndivisions];
	newOffsets = new uint64_t[ndivisions];
	chunksPerProc = new int[p];
	memset(chunksPerProc, 0, sizeof(int)*p);

	if (mpirank == 0) printf("%d: Setting offsets\n", mpirank);

	int rank = 0;
	for (unsigned int i = 0; i < ndivisions; i++) {
		offsets[i] = i*(fileLength/ndivisions);

		if (i == ndivisions-1) {
			chunkLengths[i] = fileLength - (ndivisions-1)*(fileLength/ndivisions);
			printf("Setting chunk %d's length to %ld\n", i, chunkLengths[i]);
		}
		else {
			chunkLengths[i] = fileLength/ndivisions;
			printf("Setting chunk %d's length to %ld\n", i, chunkLengths[i]);
		}

		if (rank % p == mpirank) {
			printf("%d: Assigned chunk #%d\n", mpirank, i);
			myChunks.push_back(i);
		}

		chunksPerProc[rank%p]++;
		rank++;
	}
	if (mpirank == 0) {
		printf("Chunks per proc: \n");
		for (int i = 0; i < p; i++) {
			printf("proc #%d: %d, ", i, chunksPerProc[i]);
		}
		printf("\n");
	}

	uint64_t myFreqMap[256] = {0}, globalFreqMap[256] = {0};
	memset(myFreqMap, 0, 256*sizeof(uint64_t));
	memset(globalFreqMap, 0, 256*sizeof(uint64_t));

	for (auto it = myChunks.begin(); it != myChunks.end(); it++) {
		int i = *it;
		printf("%d: Reading chunk %d from file at offset %ld with chunk size %ld\n",
				mpirank, i, offsets[i], chunkLengths[i]);
		chunks[i] = readFile(filename, chunkLengths[i], offsets[i]);
		HuffmanEncoder::frequencyMapFromText(chunks[i], myFreqMap);
	}

	int err = MPI_Allreduce(myFreqMap, globalFreqMap, 256, MPI_UINT64_T, MPI_SUM,
			MPI_COMM_WORLD);
	if (err) {
		printf("Problem with MPI all reduce. Exiting...\n");
		exit(1);
	}

	HuffmanTree *coding_tree = HuffmanEncoder::huffmanTreeFromFrequencyMap(globalFreqMap);
	vector<string> encoder = HuffmanEncoder::huffmanEncodingMapFromTree(coding_tree);

	for (auto it = myChunks.begin(); it != myChunks.end(); it++) {
		compressedChunks[*it] = HuffmanEncoder::toBinary(chunks[*it], encoder);
		newLengths[*it] = (uint64_t)ceil((double)compressedChunks[*it].size()/8);
		newLengths[*it] += sizeof(size_t);
		newOffsets[*it] = 0;
	}
	metadataOffset = calculateMetaDataSize(encoder);
	if (mpirank == 0) {
		printf("0: Metadata offset: %d\n", metadataOffset);
	}
	string output_file_name = filename + ".hez";
	if (mpirank == 0) {
		int fd = creat(output_file_name.c_str(), 0664);
		if (fd < 0) {
			printf("%d: Couldn't create output file %s\n", mpirank, output_file_name.c_str());
			exit(1);
		}
		close(fd);
	}

	HuffmanEncoder::CalculateOffsets();
	if (mpirank == 0) {
		/* Last rank knows how big the final file is, so it will take care of
		 * allocating it. Here, offset is the total length of the compressed
		 * file. */
		uint64_t compressedSize = metadataOffset;
		for (unsigned int i = 0; i < ndivisions; i++) {
			compressedSize += newLengths[i];
		}
		err = truncate(output_file_name.c_str(), compressedSize);
		if (err) {
			printf("Issue with truncate. Error: %s\n", strerror(errno));
			return (1);
		}
	}
	/* Make sure file is truncated first. */
	MPI_Barrier(MPI_COMM_WORLD);

	FILE *output_file = fopen(output_file_name.c_str(), "r+");
	if(output_file == NULL) {
		printf("%d: Couldn't open output_file %s!!!\n", mpirank, output_file_name.c_str());
		exit(1);
	}

	if (mpirank == 0) {
		printf("%d: Writing metadata!\n", mpirank);
		CompressedFile::WriteMetadataToFile(output_file, encoder);
	}

	//printf("%d: Writing my chunk to the file!\n", mpirank);
	for (auto it = myChunks.begin(); it != myChunks.end(); it++) {
		CompressedFile::WriteToFile(output_file, compressedChunks[*it], newOffsets[*it]);
		printf("%d: Wrote chunk %d to the file at offset %ld!\n", mpirank,
				*it, newOffsets[*it]);
	}
	fclose(output_file);

	return 0;
}

	void
HuffmanEncoder::CalculateOffsets()
{
	/* Keep a buffer that will store alternating pieces of information: chunk
	 * number and the size of the chunk. This can be used to calculate the
	 * cumulative offset for each chunk. */
	uint64_t *sendbuf = new uint64_t[chunksPerProc[mpirank]*2];
	int j = 0;
	for (unsigned int i = 0; i < myChunks.size(); i++) {
		/* Chunk ID */
		sendbuf[j++] = myChunks[i];
		/* Chunk length */
		sendbuf[j++] = newLengths[myChunks[i]];
	}
	printf("%d: Sendbuf: \n", mpirank);
	for (int i = 0; i < chunksPerProc[mpirank]*2; i++) {
		printf("%ld ", sendbuf[i]);
	}
	printf("\n");


	int *recvcounts = new int[p];
	int *rdispls = new int[p];

	/* Determine how much data we get from each proc */
	int displ = 0;
	for (int i = 0; i < p; i++) {
		recvcounts[i] = 2*chunksPerProc[i];
		rdispls[i] = displ;
		displ += recvcounts[i];
		printf("%d: displ: %d\n", mpirank, displ);
	}
	printf("%d: final displ: %d\n", mpirank, displ);
	uint64_t *recvbuf = new uint64_t[displ];

	/* Gather lengths of each compressed chunk */
	int err = MPI_Allgatherv(sendbuf, chunksPerProc[mpirank]*2, MPI_UINT64_T,
			recvbuf, recvcounts, rdispls, MPI_UINT64_T, MPI_COMM_WORLD);
	if (err) {
		printf("Error with MPI all gatherv. \n");
		exit(1);
	}
	printf("%d: Recvbuf: \n", mpirank);
	for (int i = 0; i < displ; i++) {
		printf("%ld ", recvbuf[i]);
	}
	printf("\n");

	/* Calculate file offsets for every compressed chunk. */
	printf("%d: Metadata offset: %d\n", mpirank, metadataOffset);
	for (int i = 0; i < displ; i++) {
		if (i % 2) {
			int chunkid = recvbuf[i-1];
			newLengths[chunkid] = recvbuf[i];
		}
	}
	uint64_t prefixSum = 0;
	for (unsigned int i = 0; i < ndivisions; i++) {
		prefixSum += newLengths[i];
		printf("Setting newOffsets[%d] = %ld\n", i, prefixSum + metadataOffset - newLengths[i]);
		newOffsets[i] = prefixSum + metadataOffset - newLengths[i];
	}
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
HuffmanEncoder::huffmanTreeFromFrequencyMap(uint64_t *FreqMap)
{
	priority_queue<HuffmanTree*,vector<HuffmanTree*>,HuffmanTreeCompare> forest{};

	for (int i = 0; i < 256; i++) {
		if (FreqMap[i] == 0) {
			continue;
		}
		HuffmanTree *tree = new HuffmanTree((char)i, FreqMap[i]);
		if (tree->GetWeight() == 0) {
			printf("Weight is 0!!!********************************************************8\n");
		}
		forest.push(tree);
	}

	printf("%ld trees in forest", forest.size());

	while (forest.size() > 1) {
		HuffmanTree *smallest = forest.top();
		forest.pop();
		HuffmanTree *secondSmallest = forest.top();
		forest.pop();
		HuffmanTree *tree = new HuffmanTree(smallest, secondSmallest);
		forest.push(tree);
	}

	return forest.top();
}

/* Pass in the initialized FreqMap. Initialize to 0 first time, then this can be
 * called on all chunks handled by the current proc. */
	void
HuffmanEncoder::frequencyMapFromText(vector<char> data, uint64_t *FreqMap)
{
	//Builds a Huffman Tree from the supplied vector of strings.
	//This function implement's Huffman's Algorithm as specified in page
	//456 of the book.
	unordered_map<char, uint64_t> freqMap{};
	for (auto it = data.begin(); it != data.end(); ++it) {
		freqMap[*it]++;
	}

	for (auto it = freqMap.begin(); it != freqMap.end(); ++it) {
		FreqMap[(int)(unsigned char)it->first] += it->second;
	}
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

	//printf("%d: encoded %ld, pushed %ld bits, size: %ld\n",mpirank, num_encoded, bits, result.size());


	return result;
}
