#!/bin/bash

#scriptsdir=.
#fanmoddir=fanmod_df
#igraphdir=igraph_df

#thisdir=`pwd`
#cd $fanmoddir
#fanmodfiles=`ls *.fanmod`
#cd $thisdir

#for fname in $fanmodfiles
#do
#igname=${fname%.fanmod}.igraph
#fanmodfile_in=$fanmoddir/$fname
#igraph_outbase=$igraphdir/$igname

#thisdir=/nfs0/BPP/Megraw_Lab/mitra/scripts/graph_utils/convert_fan_to_igraph
graphname=$1
graphoutdir=$2
fanmod_infile=$graphoutdir/$graphname.fanmod
igraph_outbase=$graphoutdir/$graphname.igraph

./fanmod_to_igraph.pl $fanmod_infile $igraph_outbase
#done
