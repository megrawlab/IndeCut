#!/bin/bash

graphname=$1
ingraph_dir=$2
seedfile=$3
igraphdir=$4
igraphroutdir=$5
num=$6

ingraphbase=$ingraph_dir/$graphname
edgefile=$ingraphbase.igraph.edges.txt
vertexfile=$ingraphbase.igraph.vertices.txt

inputSeed=`./getInputSeed.pl $seedfile $num`
edgefileout=$graphname.rand.$num.igraph
./oneRandGraphDrawWithSwapsSeeded.sh $edgefile $vertexfile $igraphdir/$edgefileout $igraphroutdir/$edgefileout.Rout $inputSeed
