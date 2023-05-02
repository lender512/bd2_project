#!/bin/bash
cmake . 
make 
# for loop 10
for i in {1..10}
do
    rm -rf ./video.dat
    touch ./video.dat
    python3 shuffle.py
    stdbuf -o0 ./bd2_project > ./log$i.csv
done

 