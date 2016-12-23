#!/bin/bash

graphname=$1
ngraphs=$2
commandsfile=$3
outdir=$4
graphsindir=$5
ncpu=`nproc`

./prepare_commands.sh $graphname $ngraphs $commandsfile $outdir $graphsindir 
#echo "commands provides for $graphname , $ngraphs, $commandsfile "

count=1
while IFS='' read -r line || [[ -n "$line" ]]; do
    #echo "Execute $line .."
    bash $line &
    let "count=count+1"
    if [ $count -gt $ncpu ]; then
	wait
	count=0
    fi
done < "$3"

#echo "All commands ran under file: $commandsfile"
