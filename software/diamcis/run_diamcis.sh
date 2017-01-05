#!/bin/bash

prog=source-code/diamcis_linux
ingraph_dir=../graphs

graphname=$1
ngraphs=$2

outdir=di_out/$graphname.$ngraphs

if [ ! -d $outdir ]
then
    mkdir -p $outdir
fi


# generate diamcis-format compatible graphs
maingraph=$ingraph_dir/$graphname.fanmod
diamcis_graph=$ingraph_dir/$graphname.diamcis

tmpgraph=tmp.igraph.txt
cut -f1,2 $maingraph > $tmpgraph
./igraph_to_diamcis.pl $tmpgraph $diamcis_graph
rm -f $tmpgraph

outfile=$outdir/$graphname.rand.diamcis.txt
timerec=$outdir/$graphname.time.txt

date > $timerec

$prog $diamcis_graph $outfile probtmp.txt $ngraphs >> $outdir/stderr.out
rm probtmp.txt
rm gmon.out

date >> $timerec

outfilesdir=$outdir/$graphname.rand.diamcis

if [ ! -d $outfilesdir ];then
        mkdir -p $outfilesdir
fi

outfilebase=$graphname.rand.diamcis

./split_diamcis.pl $outfile $outfilesdir $outfilebase

