#!/bin/bash

thisdir=`pwd`
prog=$thisdir/source-code/fanmod_linux
ingraph_dir=$thisdir/../graphs

graphname=$1
ngraphs=$2
input_graph=$ingraph_dir/$graphname.fanmod
outdir=$thisdir/fn_out/$graphname.$ngraphs

if [ ! -d $outdir ]
then
    mkdir -p $outdir
fi

motifsize=3
nsamps=100000
full_enum=1
infilename=$infile
directed=1
colored_vertices=1
colored_edges=0
random_type=0
regard_vertex_colors=1
regard_edge_colors=0
reest_subgraph_num=0
nrandnets=$ngraphs
nexchanges_per_edge=3
nexchange_attempts_per_edge=3
outfile_format=0
create_dumpfile=0

outfile=$outdir/$graphname.rand.fanmod.txt
timerec=$outdir/$graphname.time.txt

date > $timerec

$prog $motifsize $nsamps $full_enum $input_graph $directed $colored_vertices $colored_edges $random_type $regard_vertex_colors $regard_edge_colors $reest_subgraph_num $nrandnets $nexchanges_per_edge $nexchange_attempts_per_edge $outfile $outfile_format $create_dumpfile

date >> $timerec

outfilesdir=$outdir/$graphname.rand.fanmod

if [ ! -d $outfilesdir ];then
	mkdir -p $outfilesdir
fi

outfilebase=$graphname.rand.fanmod

outfilegraphs=$outdir/$graphname.rand.fanmod.txt.randgraphs
./split_fanmod-onefile.pl $outfilegraphs $outfilesdir $outfilebase
