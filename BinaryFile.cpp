#include <string.h>
#include "BinaryFile.h"

void
writeEncodingMapToFile(const vector<string>& huffmanMap, ofstream& out)
{
	/* The map will take up 2*alphabet size, so each character in the alphabet
	 * gets 2 bytes, the 1st counts the number of bits in the code, and the 2nd
	 * contains the actual bits. We are encoding at the byte level, so our
	 * alphabet is of size 256.*/

	int length = stoi(huffmanMap[256]);
	int size;
	int len;
	uint64_t compressed;
	int index;
	void *toWrite;

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
		printf("Length not correct. Length = %d\n", length);
		exit(1);
	}
	toWrite = malloc(size);
	memset(toWrite, 0, size);

	for (int i = 4; i < 256; i++) {
		/* Write at appropriate spot in array */
		index = i*2;
		string code = huffmanMap[i];

		/* Write length of code */
		switch(len) {
			case 8:
				((uint8_t*)toWrite)[index++] = code.length();
				break;
			case 16:
				((uint16_t*)toWrite)[index++] = code.length();
				break;
			case 32:
				((uint32_t*)toWrite)[index++] = code.length();
				break;
			case 64:
				((uint64_t*)toWrite)[index++] = code.length();
				break;
		}
		compressed = 0;
		/* Convert code from chars to bits */
		for (unsigned int j = 0; j < code.length(); j++) {
			compressed <<= 1;
			if (code[j] == '1') {
				compressed |= 1;
			}
		}
		/* Write code */
		switch(len) {
			case 8:
				((uint8_t*)toWrite)[index]= (uint8_t)compressed;
				break;
			case 16:
				((uint16_t*)toWrite)[index]= (uint16_t)compressed;
				break;
			case 32:
				((uint32_t*)toWrite)[index]= (uint32_t)compressed;
				break;
			case 64:
				((uint64_t*)toWrite)[index]= compressed;
				break;
		}
	}
	/* Write #bits needed to store the max code length */
	out.write((const char*)&len, (int)sizeof(int));
	/* Write map */
	out.write((const char*)toWrite, size);
	free(toWrite);
}

void
BinaryFile::WriteToFile(const vector<bool> &content, const string &output_file_name,
		const vector<string>& huffmanMap)
{
	ofstream output_file(output_file_name.c_str());
	//ofstream output_file(output_file_name.c_str(), ofstream::binary);
	
	if (!output_file.good()) {
		printf("Could not open output file for writing!\n");
		exit (1);
	}

	writeEncodingMapToFile(huffmanMap, output_file);

	/* Keeps track of all of the chars that we will write to file */
	vector<char> to_output{};

	/* Convert strings into vector of chars */
	char buffer = (char)0;
	int buffer_counter = 0;
	for (auto it = content.begin(); it != content.end(); it++) {
		bool item = *it;
		/* Bit shift all characters in string into single character */

		if (buffer_counter == 8 * sizeof(char)) {
			/* Char full, save to vector */
			to_output.push_back(buffer);
			buffer = (char)0;
			buffer_counter = 0;
		}
		buffer = buffer << 1;
		if (item == true) {
			buffer = buffer | 1;
		}
		buffer_counter++;
	}

	/* Account for any leftover buffer */
	if (buffer_counter > 0) {
		while (buffer_counter < 8) {
			buffer = buffer << 1;
			buffer_counter++;
		}
		to_output.push_back(buffer);
	}

	/* Finally, write to file. The first entry indicates number of bits
	 * that are contained within the file */
	size_t num_bits = content.size();
	output_file.write((char *)&num_bits, sizeof(size_t));

	/* Now, write binary contents */
	for (auto it = to_output.begin(); it != to_output.end(); it++) {
		char item = *it;
		output_file.write(&item, sizeof(char));
	}
	output_file.close();
}

void
readEncodingMapFromFile(vector<string>** huffmanMap, ifstream& in)
{
	void *toRead;
	int length, len, size;
	uint64_t c;
	vector<string> *hm = new vector<string>(257);

	in.read((char*)(&len), (int)sizeof(int));

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

	in.read((char*)toRead, size);
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
		(*hm)[i/2] = s;
	}
	(*hm)[256] = std::to_string(len);
	*huffmanMap = hm;
}

/* Reads a file written with WriteToFile into a vector of bools */
vector<bool>
BinaryFile::ReadFromFile(const string &input_file_name, vector<string>** huffmanMap)
{
	vector<bool> result{};
	//ifstream input_file{ input_file_name, ofstream::binary };
	ifstream input_file(input_file_name);

	if (!input_file.good()) {
		printf("Couldn't open map file for reading! \n");
		exit(1);
	}
	readEncodingMapFromFile(huffmanMap, input_file);

	/* Pick off number of bits to read */
	size_t bits_to_read = 0;
	input_file.read((char *)&bits_to_read, sizeof(size_t));

	/* Now, pull in single chars */
	char single_char = '\0';
	size_t bits_read = 0;
	for (unsigned int i = 0; i < bits_to_read; i += 8 * sizeof(char)) {
		input_file.read(&single_char, sizeof(char));

		for (unsigned int j = 0;
		    j < 8 * sizeof(char) && bits_read < bits_to_read;
		    j++) {
			/* Peel off one char, then shift to get the next one
			 * for the loop */
			char bool_val = single_char >> 8 * sizeof(char);
			bool_val = bool_val & 1;
			result.push_back((bool)bool_val);
			single_char = single_char << 1;
			bits_read++;
		}
	}

	input_file.close();

	return result;
}
