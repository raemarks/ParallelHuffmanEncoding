#!/bin/bash

./a.out compress pin/savio.txt
./a.out decompress pin/savio.pa2c pin/savio1.txt
echo ""
echo "md5sum comparisons:"
md5sum pin/savio.txt
md5sum pin/savio1.txt
echo ""
echo "stat comparisons:"
stat pin/savio.txt
stat pin/savio1.txt

#./a.out compress pin/t.txt
#./a.out decompress pin/t.pa2c pin/t1.txt
#echo ""
#echo "md5sum comparisons:"
#md5sum pin/t.txt
#md5sum pin/t1.txt
#echo ""
#echo "stat comparisons:"
#stat pin/t.txt
#stat pin/t1.txt

#./a.out compress pin/kennedy.txt
#./a.out decompress pin/kennedy.pa2c pin/kennedy1.txt
#echo ""
#echo "md5sum comparisons:"
#md5sum pin/kennedy.txt
#md5sum pin/kennedy1.txt
#echo ""
#echo "stat comparisons:"
#stat pin/kennedy.txt
#stat pin/kennedy1.txt

#./a.out compress pin/test.txt
#./a.out decompress pin/test.pa2c pin/test1.txt
#echo ""
#echo "md5sum comparisons:"
#md5sum pin/test.txt
#md5sum pin/test1.txt
#echo ""
#echo "stat comparisons:"
#stat pin/test.txt
#stat pin/test1.txt

#./a.out pin/savio.txt
#./a.out pin/kennedy.txt
