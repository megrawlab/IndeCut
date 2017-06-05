#!/bin/bash

R_LIBS="igraph"; export R_LIBS
Rprog=R

edgefile=$1
vertexfile=$2
edgefileout=$3
Routput=$4
seed=$5
#$Rprog --no-save BATCH $edgefile $vertexfile $edgefileout $seed < oneRandGraphDrawWithSwapsWSWRSLSeeded_tuned_no-permute.R > $Routput 2>&1
#$Rprog --no-save BATCH $edgefile $vertexfile $edgefileout $seed < oneRandGraphDrawWithSwapsWSWRSLSeeded_tuned_no-permute.R
./oneRandGraphDrawWithSwapsWSWRSLSeeded_tuned_no-permute.R $edgefile $vertexfile $edgefileout $seed
