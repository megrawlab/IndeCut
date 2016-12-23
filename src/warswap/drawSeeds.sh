#!/bin/bash

minSeed=1
maxSeed=2147483647
startingSeed=$1
startingDraw=$2
nseedsOut=$3
seedsfileOut=$4
routput=$seedsfileOut.Rout
R --no-save BATCH $minSeed $maxSeed $startingSeed $startingDraw $nseedsOut $seedsfileOut < drawSeeds.R > $routput 2>&1
