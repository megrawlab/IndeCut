#!/bin/bash

graphname=$1
graphoutdir=$2
igraph_infile=$graphoutdir/$graphname.igraph.edges.txt
diamcis_outbase=$graphoutdir/$graphname.diamcis

./igraph_to_diamcis.pl $igraph_infile $diamcis_outbase
#done
