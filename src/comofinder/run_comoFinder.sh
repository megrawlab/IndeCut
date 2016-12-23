#!/bin/bash

#java -jar comoFinder.jar -fp /home/mitra/workspace/comoFinder/sample_input_network.txt -ms 3 -nr 4 -ens 100 -bfd rands

jarfile=comoFinder_randout.jar
inputgraph=$1
motifSize=3
nrands=$2
inoutdir=$3
graphname=$4

outdir=$inoutdir/$graphname.$nrands
if [ ! -d $outdir ];then
	mkdir -p $outdir
fi
	

#echo "Start running comoFinder .."
#echo "java -jar comoFinder_randout.jar -fp $inputgraph -ms $motifSize -ens $nrands -bfd $outdir"
java -jar $jarfile -fp $inputgraph -ms $motifSize -ens $nrands -bfd $outdir
