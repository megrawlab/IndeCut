#!/bin/bash

ingraph_dir=../graphs
graphname=$1
ngraphs=$2
outdir=wr_out/$graphname.$ngraphs
ncpu=`nproc`

if [ ! -d $outdir ]
then
    mkdir -p $outdir
fi

# generate diamcis-format compatible graphs
maingraph=$ingraph_dir/$graphname.fanmod
igraph_outbase=$ingraph_dir/$graphname.igraph
./fanmod_to_igraph.pl $maingraph $igraph_outbase

# Run WaRSwap
drawSeedingSeeds=1
seedingSeedsFile=seedingSeeds.txt
baseNamesFile=baseNames.txt
command_file=commands.txt

./drawSeedingSeeds.sh $ngraphs $seedingSeedsFile
seedIndex=1
if [ -f $baseNamesFile ]; then
	rm $baseNamesFile
fi
      
base=$graphname
echo $base >> $baseNamesFile
startingSeed=`./getInputSeed.pl $seedingSeedsFile $seedIndex`
startingSeedFile=$base.startSEED.txt
echo $startingSeed > $startingSeedFile

seedFile=$base.SEEDS.txt
./drawSeedLists.sh $base $ngraphs

igraphdir=$outdir/$graphname.rand.igraph
igraphroutdir=$outdir/$graphname.rand.igraph_Rout
if [ ! -d $igraphdir ]
then
    mkdir $igraphdir
fi
if [ ! -d $igraphroutdir ]
then
    mkdir $igraphroutdir
fi

COUNT=0
for i in `seq 0 $ngraphs`; do
	echo generating random graph-$i
	./generate_randgraphs.sh $graphname $ingraph_dir $seedFile $igraphdir $igraphroutdir $i & 
	let "COUNT=COUNT+1"
	if [ $COUNT -gt $ncpu ]; then
		wait
		COUNT=0
	fi
done

generated_rands=`ls $igraphdir | wc -l `
while [ $generated_rands -lt $ngraphs ]; do
	sleep 5
	generated_rands=`ls $igraphdir | wc -l `
done
