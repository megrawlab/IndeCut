#!/bin/bash

input_net=$1 # input network can contain more than 1 layer and should be in fanmod format
layer1=$2    # layer1 number (TF=0, miR=1, and Gene=2)
layer2=$3
outfile=$4   # degree sequence of graph outfile, first line = out-degree and second line = in-degree

if [ $# -lt 4 ];then
	echo "./get_degree_seq_for_bilayer.sh [input_mutilayer_net] [from_layer] [to_layer] [outfile.csv]"
	echo " ---  from layer and to layer guide: 0 TF, 1 miR, 2 Gene"
	exit
fi

rm -f $outfile

# 1- extract edges from given layer
bilayer_file=$input_net".bilayer.txt"
awk "{split(\$0,a,\"\t\"); if (a[3]==$layer1 && a[4]==$layer2) print \$0}" $input_net > $bilayer_file

# 2- compute the in-degree and out-degrees and save them in output file (degree-sequence of graph)
cut -f 1 $bilayer_file | sort -n | uniq -c | tr -s [:space:] | cut -d ' ' -f 2 | sort -r -n | awk -vORS=, "{print \$0}" | sed 's/,$/\n/' >> $outfile
cut -f 2 $bilayer_file | sort -n | uniq -c | tr -s [:space:] | cut -d ' ' -f 2 | sort -r -n | awk -vORS=, "{print \$0}" | sed 's/,$/\n/' >> $outfile

rm -f $bilayer_file
