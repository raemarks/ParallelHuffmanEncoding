all:
	mpicxx -std=c++0x -Weffc++ -lm -g -Wall main.cpp CompressedFile.cpp HuffmanEncoder.cpp

easy:
	mpicxx -std=c++0x -lm -g -Wall main.cpp CompressedFile.cpp HuffmanEncoder.cpp

local:
	g++ -std=c++0x -g -Wall main.cpp CompressedFile.cpp HuffmanEncoder.cpp

file:
	g++ -std=c++0x -g -Wall random_generator.cpp -o fgen


run:
	mpiexec -machinefile ./machinefile4.txt -n 4 ./a.out

send:
	rsync machinefile1.txt machinefile2.txt machinefile4.txt test_huffman.sh random_generator.cpp main.cpp HuffmanTree.h HuffmanInternalNode.h CompressedFile.h HuffmanNode.h HuffmanEncoder.cpp StringSplitter.h HuffmanEncoder.h HuffmanLeafNode.h CompressedFile.cpp Constants.h Makefile mpdcleanup.sh rmarks@ssh1.eecs.wsu.edu:/net/u/rmarks/pvt/

test:
	./run_tests.sh
