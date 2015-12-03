all:
	#mpicxx -Wall -lm -o a.out main.c matrix.c random.c sample_sort.c
	g++ -std=c++0x -g -Wall main.cpp BinaryFile.cpp HuffmanEncoder.cpp


run:
	#mpiexec -machinefile ./machinefile4.txt -n 4 ./a.out 536870912 "junk.csv"
send:
	rsync machinefile1.txt machinefile2.txt machinefile4.txt main.cpp HuffmanInternalNode.cpp HuffmanInternalNode.h BinaryFile.h HuffmanNode.h HuffmanLeafNode.h BinaryFile.cpp Makefile rmarks@ssh1.eecs.wsu.edu:/net/u/rmarks/pvt/

test:
	./run_tests.sh
