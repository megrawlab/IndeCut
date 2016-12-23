#!/bin/bash

echo "./call_diamcis_one_file.sh [graphname] [ingraph_dir] [nrands] [outdir]"

inbase=$1   # diamcis only handles one color, hubex10 is a one-color example
#inbase=hubex10sep # treats this two-color example as if vertices were all one color
#inbase=simple    # treats this two-color example as if vertices were all one color
#inbase=simple2    # treats this two-color example as if vertices were all one color
indir=$2
ngraphs=$3

outbase=$inbase.$ngraphs
outdir=$4/$outbase

timerec=$outdir/$outbase.time.txt

if [ ! -d $outdir ]
then
    mkdir -p $outdir
fi

edgefile=$indir/$inbase.diamcis.edges.txt

date > $timerec

edgefileout=$outdir/$outbase.rand.diamcis.txt
./dia-mcis-standard $edgefile $edgefileout probtmp.txt $ngraphs >> $outdir/stderr.out
    
date >> $timerec

rm probtmp.txt
rm gmon.out

outfilesdir=$outdir/$outbase.rand.diamcis
outfilebase=$outbase.rand.diamcis
if [ ! -d $outfilesdir ]; then
	mkdir $outfilesdir
fi

./split_diamcis.pl $edgefileout $outfilesdir $outfilebase

inbase=$outbase
dir=$outdir
indir=$dir/$inbase.rand.diamcis
outfile=$dir/$inbase.rand.diamcis.labeled_graphs.txt
#./sortgraphs.pl $indir $outfile
