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

p=1
 
cp inputs/t1.txt inputs/t1.txt.orig
mpiexec -machinefile ./machinefile4.txt -n $p ./a.out compress inputs/t1.txt
echo "Finished"
echo ""
echo ""
mpiexec -machinefile ./machinefile4.txt -n $p ./a.out decompress inputs/t1.txt.hez
echo "Finished"
echo ""
echo ""
echo "MD5SUM COMPARISONS:"
md5sum inputs/t1.txt.orig
md5sum inputs/t1.txt
echo ""
echo "STAT COMPARISONS:"
stat inputs/t1.txt.orig
stat inputs/t1.txt
echo ""
echo "STAT OF COMPRESSED FILE:"
stat inputs/t1.txt.hez

rm -rf inputs/t1.txt inputs/t1.txt.hez
mv inputs/t1.txt.orig inputs/t1.txt

#./a.out pin/t1.txt
#./a.out pin/kennedy.txt
