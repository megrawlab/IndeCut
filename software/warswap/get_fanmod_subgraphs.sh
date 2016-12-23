#!/bin/bash

#prog=/nfs0/BPP/Megraw_Lab/megraw/software/FANMOD-command_line-source-modified/fanmodm
prog=./fanmodm

infile=$1
outfile=$2

motifsize=3
nsamps=0
full_enum=1
infilename=$infile
directed=1
colored_vertices=1
colored_edges=0
random_type=0
regard_vertex_colors=1
regard_edge_colors=0
reest_subgraph_num=0
nnets=0
nexchanges_per_edge=1
nexchange_attempts_per_edge=1
outfilename=$outfile
outfile_format=0
create_dumpfile=0

$prog $motifsize $nsamps $full_enum $infilename $directed $colored_vertices $colored_edges $random_type $regard_vertex_colors $regard_edge_colors $reest_subgraph_num $nnets $nexchanges_per_edge $nexchange_attempts_per_edge $outfilename $outfile_format $create_dumpfile
