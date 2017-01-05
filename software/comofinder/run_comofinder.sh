#!/bin/bash

prog=comofinder_randout.jar

ingraph_dir=../graphs

graphname=$1
ngraphs=$2

outdir=cf_out/$graphname.$ngraphs
randoutdir=$outdir/$graphname.rand.comofinder

if [ ! -d $outdir ]
then
    mkdir -p $outdir
fi

comofinder_graph=$ingraph_dir/$graphname.fanmod

timerec=$outdir/$graphname.time.txt

date > $timerec

java -jar $prog -fp $comofinder_graph -ms 3 -ens $ngraphs -bfd $randoutdir
rm -f finalMotif.txt

date >> $timerec
