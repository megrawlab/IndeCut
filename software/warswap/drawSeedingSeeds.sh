#!/bin/bash

nseeds=$1
outfile=$2
routput=$outfile.Rout
R --no-save BATCH $nseeds $outfile < drawSeedingSeeds.R > $routput 2>&1
