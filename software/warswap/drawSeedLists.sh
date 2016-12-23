#!/bin/bash

base=$1
nseedsOut=$2

startingSeedFile=$base.startSEED.txt
startingSeed=`./getInputSeed.pl $startingSeedFile 1`
seedsfileOut=$base.SEEDS.txt

startingDraw=1
./drawSeeds.sh $startingSeed $startingDraw $nseedsOut $seedsfileOut
