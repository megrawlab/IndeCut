#!/bin/bash
# Set the output and input directories 

if [ $# -lt 5 ]; then
	echo "./prepare_commands.sh [graphname] [no_of_graphs] [out_commandsfile] [outdir] [graphs_indir]"
	exit
fi
runName=warswapLocal

outbasedir=$4
datafiledir=$5
jobname=testUniform
nGraphs=$2
graphname=$1
commandsfile=$3

if [ ! -d $outbasedir ]; then
	mkdir $outbasedir
fi

./setup_parameters.sh $runName $outbasedir $datafiledir $jobname $nGraphs $graphname $commandsfile
