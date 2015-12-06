#!/bin/bash

#cp pin/savio.txt savio.txt.orig
#./a.out compress pin/savio.txt
#ls pin/savio.txt.hez
#./a.out decompress pin/savio.txt.hez
#echo ""
#echo "md5sum comparisons:"
#md5sum savio.txt.orig
#md5sum pin/savio.txt
#echo ""
#echo "stat comparisons:"
#stat savio.txt.orig
#stat pin/savio.txt
#
#rm -rf pin/savio.txt
#mv savio.txt.orig pin/savio.txt

cp t1.txt t1.txt.orig
./a.out compress t1.txt
ls t1.txt.hez
./a.out decompress t1.txt.hez
echo ""
echo "md5sum comparisons:"
md5sum t1.txt.orig
md5sum t1.txt
echo ""
echo "stat comparisons:"
stat t1.txt.orig
stat t1.txt

rm -rf t1.txt
mv t1.txt.orig t1.txt

#./a.out pin/t1.txt
#./a.out pin/kennedy.txt
