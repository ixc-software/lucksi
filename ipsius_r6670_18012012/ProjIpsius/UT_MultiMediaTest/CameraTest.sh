#!/bin/sh

echo "Start camera test..." > ../test.txt

if [ -f ./UT_MultiMediaTest/UT_MultiMedia_test_800_480 ] ; then
	./UT_MultiMediaTest/UT_MultiMedia_test_800_480
else
	echo "UT_MultiMedia_test not found ..." >> ../test.txt

fi 


#echo "Stop camera test..." >> ../test.txt

