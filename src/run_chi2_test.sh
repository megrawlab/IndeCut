#!/bin/bash

if [ $# -lt 5 ]; then
        echo "Usage :  ./run_chi2_test.sh [graph_in_dir] [source nodes count] [target nodes count] [output_file] [TRUE | FALSE]* "
	echo "* is bilayer graph?"
        exit 1
fi

indir=`readlink -f $1`
srcNodesNo=$2
tgtNodesNo=$3
outputfile=`readlink -f $4`
isBilayer=$5
nsamples=`ls -l $indir | wc -l`

temp_file=$outputfile.log
rm -f $temp_file

java -Xms1024m -Xmx1024g -cp classes edu.osu.warswap.unifomity.IndependenceTest $indir $srcNodesNo $tgtNodesNo $outputfile $isBilayer $nsamples > $temp_file


