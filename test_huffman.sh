#!/bin/bash

./a.out compress pin/test.txt
./a.out decompress pin/test.pa2c pin/test1.txt
echo "md5sum comparisons:"
md5sum pin/test.txt
md5sum pin/test1.txt

#./a.out pin/savio.txt
#./a.out pin/kennedy.txt
