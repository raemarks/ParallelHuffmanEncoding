#include "BinaryFile.h"

void
BinaryFile::WriteToFile(const vector<bool> &content, const string &output_file_name)
{
	ofstream output_file(output_file_name.c_str());
	//ofstream output_file(output_file_name.c_str(), ofstream::binary);

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

/* Reads a file written with WriteToFile into a vector of bools */
vector<bool>
BinaryFile::ReadFromFile(const string &input_file_name)
{
	vector<bool> result{};
	//ifstream input_file{ input_file_name, ofstream::binary };
	ifstream input_file(input_file_name);

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
