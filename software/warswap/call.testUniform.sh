#!/bin/bash

task_id=$1
let "num = $task_id - 1"

#layerNo=
base=graph1
seedfile=graph1.SEEDS.txt

datafiledir=/home/mcb/ansariom/mitra/Projects/NetMotif/IndeCut/test_software/graphs

basedir=/home/mcb/ansariom/mitra/Projects/NetMotif/IndeCut/test_software/warswap/wr_out/$base
igraphdir=$basedir/$base.rand.igraph
igraphroutdir=$basedir/$base.rand.igraph_Rout
fanmoddir=$basedir/$base.rand.fanmod
fanmodsgdir=$basedir/$base.rand.fanmod_subgraphs
if [ ! -d $basedir ]
then
    mkdir $basedir
fi
if [ ! -d $igraphdir ]
then
    mkdir $igraphdir
fi
if [ ! -d $igraphroutdir ]
then
    mkdir $igraphroutdir
fi
if [ ! -d $fanmoddir ]
then
    mkdir $fanmoddir
fi
    if [ ! -d $fanmodsgdir ]
    then
    mkdir $fanmodsgdir
fi

edgefile=$base.igraph.edges.txt                                                        
vertexfile=$base.igraph.vertices.txt                                                   

removeSelfLoops=1

if [ "$num" -eq "0" ]
    then
    edgefileout=$base.ORIG.igraph
    fanmodfile=$edgefileout.fanmod
    ./igraph_to_fanmod.pl $removeSelfLoops $datafiledir/$vertexfile $datafiledir/$edgefile $fanmoddir/$fanmodfile

    fanmodout=$fanmodfile.out
    ./get_fanmod_subgraphs.sh $fanmoddir/$fanmodfile $fanmodsgdir/$fanmodout
    rm $fanmoddir/$fanmodfile # remove each fanmod graph file after it is used
else
    inputSeed=`./getInputSeed.pl $seedfile $num`
    edgefileout=$base.rand.$num.igraph
    ./oneRandGraphDrawWithSwapsSeeded.sh $datafiledir/$edgefile $datafiledir/$vertexfile $igraphdir/$edgefileout $igraphroutdir/$edgefileout.Rout $inputSeed

    if [ -f $igraphdir/$edgefileout ]
        then
        fanmodfile=$edgefileout.fanmod
        ./igraph_to_fanmod.pl $removeSelfLoops $datafiledir/$vertexfile $igraphdir/$edgefileout $fanmoddir/$fanmodfile

        fanmodout=$fanmodfile.out
        #./get_fanmod_subgraphs.sh $fanmoddir/$fanmodfile $fanmodsgdir/$fanmodout
        rm $fanmoddir/$fanmodfile # remove each fanmod graph file after it is used
        #rm $igraphdir/$edgefileout # remove each igraph file after it is used
    fi

fi
