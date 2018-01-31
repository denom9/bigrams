#!/bin/bash
SUM=${0}
for i in {1..10};
do SUM+=${./../cmake-build-debug/bigrammi "../files/inputs/input3.txt"};
echo $i;
done

