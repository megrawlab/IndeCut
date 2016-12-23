#!/bin/bash
#=========================================================================================
#  This script provides required input files and parameters for running FANMOD and WaRSwap
#=========================================================================================
if [ $# -lt 6 ]; then
	echo "./setup_parameters [runName] [outbasedir] [datafiledir] [jobName] [no_of_rand_graphs] [graph_basename] [output_commands_file]"
	exit 
fi

ngraphs=$5  #Number of random networks + 1 

runName=$1
outbasedir=$2
datafiledir=$3
jobname=$4
graphbasename=$6

drawSeedingSeeds=1
seedingSeedsFile=$runName.seedingSeeds.txt
baseNamesFile=$runName.baseNames.txt
command_file=$7

if [ ! -d $outbasedir ]
then
    mkdir $outbasedir
fi

bases=( $graphbasename )

useExistingSeedListsArray=0
useExistingSeedListsAll=0
useExistingSeedLists=( 1 1 1 0 0 0 0 0)

element_count=${#bases[@]}

if [ "$drawSeedingSeeds" -eq "1" ]
    then
    ./drawSeedingSeeds.sh $element_count $seedingSeedsFile
    index=0
    seedIndex=1
    if [ -f $baseNamesFile ]
	then
	rm $baseNamesFile
    fi
    while [ "$index" -lt "$element_count" ]
      do
      base=${bases[$index]}
      echo $base >> $baseNamesFile
      startingSeed=`./getInputSeed.pl $seedingSeedsFile $seedIndex`
      startingSeedFile=$base.startSEED.txt
      echo $startingSeed > $startingSeedFile
      let "index = $index + 1"
      let "seedIndex = $seedIndex + 1"
    done
    
else
    index=0
    seedIndex=1
    while [ "$index" -lt "$element_count" ]
      do
      base=${bases[$index]}
      startingSeed=`./getInputSeed.pl $seedingSeedsFile $seedIndex`
      startingSeedFile=$base.startSEED.txt
      echo $startingSeed > $startingSeedFile
      let "index = $index + 1"
      let "seedIndex = $seedIndex + 1"
    done    
fi

callfile=call.$jobname.sh   #The bash file which will be run in cluster

index=0
while [ "$index" -lt "$element_count" ]
  do
  base=${bases[$index]}
  useExistingSeedList=${useExistingSeedLists[$index]}
  
  echo "Starting $base"
  date
  seedFile=$base.SEEDS.txt
  
  if [ "$useExistingSeedListsArray" -eq "1" ]
      then

      if [ "$useExistingSeedList" -eq "0" ]
	  then
	  ./drawSeedLists.sh $base $ngraphs
      fi

      else

      if [ "$useExistingSeedListsAll" -eq "0" ]
	  then
	  ./drawSeedLists.sh $base $ngraphs
      fi

  fi

  # dynamically generation of call_file
  ./write_call_file.pl $base $outbasedir $datafiledir $seedFile $callfile
  chmod +x $callfile
  
  # Write commands into a file and give this list of commands to Jason's submit script to control submission load
  njobs=1
  rm -f $command_file
  while [ $njobs -le $ngraphs ]
  do

    echo "./"$callfile" "$njobs >> $command_file
    let "njobs = $njobs + 1"
  done
  
  let "index = $index + 1"
done
