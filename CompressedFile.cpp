#include <string.h>
#include "CompressedFile.h"
#include "Constants.h"

void
writeEncodingMapToFile(const vector<string>& huffmanMap, FILE *out)
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

	for (int i = 0; i < 256; i++) {
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
	fwrite((const char*)&len, sizeof(int), 1, out);
	/* Write map */
	fwrite((const void*)toWrite, 1, size, out);
	free(toWrite);
}

void
writeDvisionsToFile(FILE *out)
{
	/* TODO: make sure only proc 0 does this. */
	printf("Writing divisions...\n");
	fwrite((void*)&ndivisions, sizeof(uint32_t), 1, out);
	fwrite((void*)offsets, sizeof(uint64_t), ndivisions, out);
	printf("%d divisions\n", ndivisions);
}

/* Offset is realtive to the beginning of the file, so the caller must make sure
 * to adjust for the metadata at the beginning of the file. */
void
CompressedFile::WriteToFile(const vector<bool> &content, FILE *output_file,
		const vector<string>& huffmanMap)
{
	writeDvisionsToFile(output_file);
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
	fwrite((void *)&num_bits, sizeof(size_t), 1, output_file);

	/* Now, write binary contents */
	for (auto it = to_output.begin(); it != to_output.end(); it++) {
		char item = *it;
		fwrite(&item, sizeof(char), 1, output_file);
	}
}

void
readEncodingMapFromFile(vector<string>** huffmanMap, FILE *in)
{
	void *toRead;
	int length, size;
	uint32_t len;
	uint64_t c;
	vector<string> *hm = new vector<string>(257);

	fread((void*)(&len), sizeof(len), 1, in);

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

	fread((void*)toRead, 1, size, in);
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
	(*hm)[256] = std::to_string((long long int)len);
	*huffmanMap = hm;
}

void
readDvisionsFromFile(FILE *in)
{
	/* TODO: make sure only proc 0 does this. */
	printf("Reading divisions...\n");
	fread((void*)&ndivisions, sizeof(uint32_t), 1, in);
	offsets = (uint64_t*) malloc(sizeof(uint64_t)*ndivisions);
	fread((void*)offsets, sizeof(uint64_t), ndivisions, in);
	printf("%d divisions\n", ndivisions);
}

/* Reads a file written with WriteToFile into a vector of bools. Precondition:
 * input_file is open and good, and caller must close it. Offset is relative to
 * the beginning of the file, so it must be adjusted not to write over the
 * metadata by the caller. */
vector<bool>
CompressedFile::ReadFromFile(FILE *input_file, vector<string>** huffmanMap, uint64_t offset)
{
	vector<bool> result{};

	readDvisionsFromFile(input_file);
	readEncodingMapFromFile(huffmanMap, input_file);

	/* Pick off number of bits to read */
	size_t bits_to_read = 0;
	fread((void *)&bits_to_read, sizeof(size_t), 1, input_file);

	/* Now, pull in single chars */
	char single_char = '\0';
	size_t bits_read = 0;
	for (unsigned int i = 0; i < bits_to_read; i += 8 * sizeof(char)) {
		fread((void*)&single_char, sizeof(char), 1, input_file);

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

	return result;
}
