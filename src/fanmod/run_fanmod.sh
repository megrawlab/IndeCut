#!/bin/bash                                                                                                      

if [ $# -lt 3 ]; then
	echo "USAGE: ./run_fanmod.sh [INPUT_GRAPH_DIR] [INGRAPH_BASE] [No of BG Nets] [out_director]"
	exit 
fi

inbase=$2
indir=$1
ngraphs=$3


outbase=$inbase.$ngraphs
outdir=$4/$outbase
timerec=$outdir/$outbase.time.txt

if [ ! -d $outdir ]
then
    mkdir -p $outdir
fi

infile=$inbase.fanmod
outfile=$outdir/$outbase.rand.fanmod.txt

date > $timerec

./get_fanmod_outputmultirandgraphs.sh $indir/$infile $outfile $ngraphs

date >> $timerec

outfilesdir=$outdir/$outbase.rand.fanmod
outfilebase=$outbase.rand.fanmod
mkdir $outfilesdir

outfilegraphs=$outdir/$outbase.rand.fanmod.txt.randgraphs
./split_fanmod-onefile.pl $outfilegraphs $outfilesdir $outfilebase

inbase=$outbase
dir=$outdir
indir=$dir/$inbase.rand.fanmod
outfile=$dir/$inbase.rand.fanmod.labeled_graphs.txt
