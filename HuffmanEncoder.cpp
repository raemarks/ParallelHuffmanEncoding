#include <string.h>
#include <cstdio>
#include <stdint.h>
#include <queue>
#include "HuffmanEncoder.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

struct HuffmanTreeCompare {
	bool operator()(HuffmanTree *a, HuffmanTree *b){
		return a->GetWeight() > b->GetWeight();
	}
};

#if 0
static vector<string>
readFile(string input_file_name)
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
#endif

void printChar(char c)
{
	printf("%d", (int) c);
}

	HuffmanTree*
HuffmanEncoder::huffmanTreeFromText(vector<string> data)
{
	//Builds a Huffman Tree from the supplied vector of strings.
	//This function implement's Huffman's Algorithm as specified in page
	//456 of the book.
	unordered_map<char, int> freqMap;

	for (auto it = data.begin(); it != data.end(); ++it) {
		string s = *it;
		for (auto i = s.begin(); i != s.end(); ++i) {
			if ((int) *i < 0) {
				//printf("Less than 0 in freq map!!\n");
			}
			if (*i == 1) {
				printf("Got 1!\n");
			}
			freqMap[*i]++;
		}
	}

	priority_queue<HuffmanTree*,vector<HuffmanTree*>,HuffmanTreeCompare> forest{};

	for (auto it = freqMap.begin(); it != freqMap.end(); it++) {
		HuffmanTree *tree = new HuffmanTree(it->first, it->second);
		forest.push(tree);
	}

	while (forest.size() > 1) {
		HuffmanTree *smallest = forest.top();
		forest.pop();
		HuffmanTree *secondSmallest = forest.top();
		forest.pop();
		HuffmanTree *tree = new HuffmanTree(smallest, secondSmallest);
		forest.push(tree);
	}
	//forest.top()->Print();

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

	for (int i = 0; i < huffmanMap.size() - 2; i++) {
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
	inFile.read((char*)(&len), (int)sizeof(int));

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
	HuffmanNode *n = tree->GetRoot();
	ostringstream result{};
	char last_char = 0;
	uint64_t last_index = 0;
	string stack;

	printf("Tree:\n");
	tree->Print();


	for (auto it = bits.begin(); it != bits.end(); ++it) {
		bool bit = *it;
		if (n == NULL) {
			printf("NULL!\n"); exit(1);
		}
		if (bit == false) {
			stack += "0";
			n = ((HuffmanInternalNode*)n)->GetLeftChild();
		}
		else {
			stack += "1";
			n = ((HuffmanInternalNode*)n)->GetRightChild();
		}
		if (n == NULL) {
			printf("N was null! Last char: %c at index %ld, stack = %s\n", last_char, last_index, stack.c_str());
		}
		if (n->IsLeaf()) {
			result << ((HuffmanLeafNode*)n)->GetValue();
			last_char =  ((HuffmanLeafNode*)n)->GetValue();
			n = tree->GetRoot();
			stack = "";
		}
		last_index++;
	}

	return result.str();
}

	vector<bool>
HuffmanEncoder::toBinary(vector<string> text, vector<string> huffmanMap)
{
	vector<bool> result;

	for (auto it = text.begin(); it != text.end(); ++it) {
		string s = *it;

		for (auto it2 = s.begin(); it2 != s.end(); ++it2) {
			char c = *it2;
			string code = huffmanMap[(int)((unsigned char)c)];

			for (auto it3 = code.begin(); it3 != code.end(); ++it3) {
				char b = *it3;
				result.push_back(b == '1');
			}
		}
	}

	return result;
}
