#include <cstdio>
#include <stdint.h>
#include <queue>
#include "HuffmanEncoder.h"

struct HuffmanTreeCompare {
	bool operator()(HuffmanTree *a, HuffmanTree *b){
		return a->GetWeight() > b->GetWeight();
	}
};

void printChar(char c)
{
	printf("%d", (int) c);
}

	HuffmanTree*
HuffmanEncoder::huffmanTreeFromText(vector<string> data)
{
	printf("Entering huffmanTreeFromText\n");
	//Builds a Huffman Tree from the supplied vector of strings.
	//This function implement's Huffman's Algorithm as specified in page
	//456 of the book.
	unordered_map<char, int> freqMap;

	for (auto it = data.begin(); it != data.end(); ++it) {
		string s = *it;
		for (auto i = s.begin(); i != s.end(); ++i) {
			freqMap[*i]++;
		}
	}

	priority_queue<HuffmanTree *,
		vector<HuffmanTree*>,
		HuffmanTreeCompare> forest;

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
	printf("Tree created from text:\n");
	forest.top()->Print();

	printf("Exiting huffmanTreeFromText\n");
	return forest.top();
}

	HuffmanTree*
HuffmanEncoder::huffmanTreeFromMap(string* huffmanMap)
{
	printf("Entering huffmanTreeFromMap\n");
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

		for (unsigned int i = 0; i < code.length() -1; ++i) {
			if (code[i] == '0') {
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

	printf("Tree created from map:\n");
	tree->Print();

	printf("Exiting huffmanTreeFromMap\n");
	return tree;
}

	void
recursiveBuildMapFromTree(HuffmanNode *n, string s, string* m)
{
	if (n->IsLeaf()) {
		//Base case
		HuffmanLeafNode *ln = (HuffmanLeafNode*) n;
		m[(int)ln->GetValue()] = s;
		return;
	}
	else {
		HuffmanInternalNode *in = (HuffmanInternalNode*) n;

		if (in->GetLeftChild() != NULL) {
			recursiveBuildMapFromTree(in->GetLeftChild(), s + "0", m);
		}
		if (in->GetRightChild() != NULL) {
			recursiveBuildMapFromTree(in->GetRightChild(), s + "1", m);
		}
		return;
	}
}

	string*
HuffmanEncoder::huffmanEncodingMapFromTree(HuffmanTree *tree)
{
	printf("Entering encodingMapFromTree\n");
	//Generates a Huffman Map based on the supplied Huffman Tree.  Again, recall
	//that a Huffman Map contains a series of codes(e.g. 'a' = > 001).Each digit(0, 1)
	//in a given code corresponds to a left branch for 0 and right branch for 1.
	//As such, a given code represents a pre-order traversal of that bit of the
	//tree.  I used recursion to solve this problem.
	string* huffmanMap = new string[256];
	string s = "";

	recursiveBuildMapFromTree(tree->GetRoot(), s, huffmanMap);

	printf("Exiting encodingMapFromTree\n");
	return huffmanMap;
}

	void
HuffmanEncoder::writeEncodingMapToFile(string* huffmanMap, string file_name)
{
	printf("Entering writeEncodingMapToFile\n");
	/* The map will take up 2*alphabet size, so each character in the alphabet
	 * gets 2 bytes, the 1st counts the number of bits in the code, and the 2nd
	 * contains the actual bits. We are encoding at the byte level, so our
	 * alphabet is of size 256.*/

	ofstream outFile(file_name.c_str());
	//ofstream outFile(file_name.c_str(), ofstream::binary);
	char toWrite[512], c;
	int index;

	if (!outFile.good()) {
		printf("Couldn't open map file for writing! \n");
		exit(1);
	}

	for (int i = 0; i < 256; i++) {
		/* Write at appropriate spot in array */
		index = i*2;
		string code = huffmanMap[i];


		/* Write length of code */
		toWrite[index++] = (char) code.length();
		c = 0;
		/* Convert code from chars to bits */
		for (unsigned int j = 0; j < code.length(); j++) {
			c <<= 1;
			if (code[j] == '1') {
				c |= 1;
			}
		}
		/* Write code */
		toWrite[index]= c;
	}
	outFile.write(toWrite, 512);
	outFile.close();
	printf("Exiting writeEncodingMapToFile\n");
}

	string*
HuffmanEncoder::readEncodingMapFromFile(string file_name)
{
	printf("Entering readEncodingMapFromFile\n");
	ifstream inFile(file_name.c_str());
	//ifstream inFile(file_name.c_str(), ifstream::binary);
	char toRead[512];
	string* huffmanMap = new string[256];

	if (!inFile.good()) {
		printf("Couldn't open map file for reading! \n");
		exit(1);
	}
	inFile.read(toRead, 512);
	inFile.close();
	for (unsigned int i = 0; i < 512; i+=2) {
		int length = (int) toRead[i];
		char c = toRead[i+1];

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

	printf("Exiting readEncodingMapFromFile\n");
	return huffmanMap;
}

	string
HuffmanEncoder::decodeBits(vector<bool> bits, string* huffmanMap)
{
	printf("Entering decodeBits, decompressing %ld bits\n", bits.size());
	HuffmanTree *tree = HuffmanEncoder::huffmanTreeFromMap(huffmanMap);
	HuffmanNode *n = tree->GetRoot();
	ostringstream result{};


	for (auto it = bits.begin(); it != bits.end(); ++it) {
		bool bit = *it;
		if (bit == false) {
			n = ((HuffmanInternalNode*)n)->GetLeftChild();
		}
		else {
			n = ((HuffmanInternalNode*)n)->GetRightChild();
		}
		if (n->IsLeaf()) {
			result << ((HuffmanLeafNode*)n)->GetValue();
			printf("Decoded %d\n", (int) ((HuffmanLeafNode*)n)->GetValue());
			n = tree->GetRoot();
		}
	}

	printf("Exiting decodeBits\n");
	return result.str();
}

	vector<bool>
HuffmanEncoder::toBinary(vector<string> text, string* huffmanMap)
{
	printf("Entering toBinary\n");
	vector<bool> result;

	for (auto it = text.begin(); it != text.end(); ++it) {
		string s = *it;

		for (auto it2 = s.begin(); it2 != s.end(); ++it2) {
			char c = *it2;
			string code = huffmanMap[(int)c];

			for (auto it3 = code.begin(); it3 != code.end(); ++it3) {
				char b = *it3;
				result.push_back(b == '1');
			}
			printf("Encoded %d\n", (int) c);
		}
	}

	printf("Exiting toBinary, encoded %ld bits\n", result.size());
	return result;
}
