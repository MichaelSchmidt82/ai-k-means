#!/bin/bash
clusters=$1

num_of_attributes=4
data_file="datasets/iris-data.txt"

for ((seed=1;seed<=100;seed++)); do cat $data_file | ./split.sh 10 ./kmeans $seed $clusters $num_of_attributes ; done >> out$1
