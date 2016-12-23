#!/bin/bash
echo "Running the tet pieline for generating and testing random bi-layer graphs"


outdir=output
if [ ! -d $outdir ];then
        mkdir $outdir
fi

PYTHON=/local/cluster/bin/python
CSDP_exe=Csdp-6.1.1/solver/csdp

graphname=graph1
src_degdist="10,1,1,1,1,1,1,1,1,1,1"
target_degdist="10,1,1,1,1,1,1,1,1,1,1"
ngraphs=5000

IFS=',' read -r -a sarr <<< "$src_degdist"
IFS=',' read -r -a tarr <<< "$target_degdist"
src_count=${#sarr[@]}
target_count=${#tarr[@]}

convert_graph_script=./fanmod_to_igraph.sh
convert_to_diamcis_script=./igraph_to_diamcis.sh
gengraphs_script=./bipartite_graph_generation.R

#####################################################################
# ---- Change thisdir to your home-relative dir ----- #
thisdir=`pwd`


wr_dir=$thisdir/warswap
fn_dir=$thisdir/fanmod
cf_dir=$thisdir/comofinder
di_dir=$thisdir/diamcis

wr_script=$wr_dir/run_warswap.sh
fn_script=$fn_dir/run_fanmod.sh
cf_script=$cf_dir/run_comoFinder.sh
di_script=$di_dir/run_diamcis.sh

wr_outdir=$wr_dir/wr_out
fn_outdir=$fn_dir/fn_out
cf_outdir=$cf_dir/cf_out
di_outdir=$di_dir/di_out

wr_randoutdir=$wr_outdir/$graphname/$graphname.rand.igraph
fn_randoutdir=$fn_outdir/$graphname.$ngraphs/$graphname.$ngraphs.rand.fanmod
cf_randoutdir=$cf_outdir/$graphname.$ngraphs
di_randoutdir=$di_outdir/$graphname.$ngraphs/$graphname.$ngraphs.rand.diamcis

#____________________________________________________________________

echo "Graph = $graphname, src_count= $src_count, target_count = $target_count"

echo "Generating graphs for FANMOD and WARSWAP with given degree dist..."
graphoutdir=$thisdir/graphs
if [ ! -d $graphoutdir ];then
	mkdir -p $graphoutdir
fi
goutfile=$graphoutdir/$graphname.fanmod

#echo $gengraphs_script $src_degdist $target_degdist $goutfile
$gengraphs_script $src_degdist $target_degdist $goutfile
if [ $? -ne 0 ]; then
    echo Failed to generate graph!
    exit
fi

#echo $convert_graph_script $graphname $graphoutdir
$convert_graph_script $graphname $graphoutdir
if [ $? -ne 0 ]; then
    echo Failed to generate graph!
    exit
fi

#echo $convert_to_diamcis_script $graphname $graphoutdir
$convert_to_diamcis_script $graphname $graphoutdir
if [ $? -ne 0 ]; then
    echo Failed to generate graph!
    exit
fi
echo "Graphs generated. "

cd $wr_dir
echo "Executing of WarSwap started .. $(date)"
commandsfile=commands.txt
#echo $wr_script $graphname $ngraphs $commandsfile $wr_outdir $graphoutdir
$wr_script $graphname $ngraphs $commandsfile $wr_outdir $graphoutdir
if [ $? -ne 0 ]; then
    echo Failed to run WaRSwap!
    exit
fi
echo "WaRSwap is done! $(date)"

cd $fn_dir
echo "Executing of FANMOD started .. $(date)"
$fn_script $graphoutdir $graphname $ngraphs $fn_outdir
if [ $? -ne 0 ]; then
    echo Failed to run FANMOD!
    exit
fi
echo "FANMOD is done! $(date)"

cd $cf_dir
echo "Executing of CoMoFinder started .. $(date)"
$cf_script $goutfile $ngraphs $cf_outdir $graphname
if [ $? -ne 0 ]; then
    echo Failed to run CoMoFinder!
    exit
fi
echo "CoMoFinder is done! $(date)"

cd $di_dir
echo "Executing of DIA_MCIS started .. $(date)"
#echo $di_script $graphname $graphoutdir $ngraphs $di_outdir
$di_script $graphname $graphoutdir $ngraphs $di_outdir
if [ $? -ne 0 ]; then
    echo Failed to run DIA-MCIS!
    exit
fi
echo "DIA_MCIS is done! $(date)"

#echo "IndeCut time - start: $(date)"
cd $thisdir
echo "Getting  matrix A for WaRSwap .."
wr_outfile=$outdir/$graphname"_WR"_$ngraphs.out
#echo "./run_chi2_test.sh $wr_randoutdir $src_count $target_count $wr_outfile TRUE"
./run_chi2_test.sh $wr_randoutdir $src_count $target_count $wr_outfile TRUE &
pid_1=$!
#echo "WR_E done!"

echo "Getting matrix A for FANMOD .."
fn_outfile=$outdir/$graphname"_fanmod"_$ngraphs.out
./run_chi2_test.sh $fn_randoutdir $src_count $target_count $fn_outfile TRUE &
pid_2=$!
#echo "FN_E done!"

echo "Getting matrix A for comoFinder .."
cf_outfile=$outdir/$graphname"_CF"_$ngraphs.out
#echo "./run_chi2_test.sh $cf_randoutdir $src_count $target_count $cf_outfile TRUE"
./run_chi2_test.sh $cf_randoutdir $src_count $target_count $cf_outfile TRUE &
pid_3=$!
#echo "CF_E done!"

echo "Getting matrix A for DIA-MCIS .."
di_outfile=$outdir/$graphname"_diamcis"_$ngraphs.out
#echo "./run_chi2_test.sh $di_randoutdir $src_count $target_count $di_outfile TRUE"
./run_chi2_test.sh $di_randoutdir $src_count $target_count $di_outfile TRUE &
pid_4=$!
#echo "DI_E done!"

wait $pid_1
wait $pid_2
wait $pid_3
wait $pid_4

echo "Sample Average (A) matrices computed!!"

tmp1=tmp1.csv
tmp2=tmp2.csv
zfile=$outdir/$graphname"_Z"_$ngraphs.csv
echo "Computing matrix Z started .."
echo $src_degdist >> $tmp1
echo $target_degdist >> $tmp2
echo "$PYTHON MaxEntMatrix.py -c $tmp2 -r $tmp1 -o $zfile"
$PYTHON MaxEntMatrix.py -c $tmp2 -r $tmp1 -o $zfile
rm -f $tmp1
rm -f $tmp2
if [ $? -ne 0 ]; then
    echo Failed to compute matrix Z!
    exit
fi
echo "Matrix Z computed! ( $zfile )"

echo "Computing cut norms started .."
E_suffix=_E_out.csv
wr_E_file=$wr_outfile"$E_suffix"
fn_E_file=$fn_outfile"$E_suffix"
cf_E_file=$cf_outfile"$E_suffix"
di_E_file=$di_outfile"$E_suffix"
wr_cutnorm_outfile=$wr_outfile.cutnorm.csv
fn_cutnorm_outfile=$fn_outfile.cutnorm.csv
cf_cutnorm_outfile=$cf_outfile.cutnorm.csv
di_cutnorm_outfile=$di_outfile.cutnorm.csv

#echo "$PYTHON CutnormApprox.py -m $zfile -s $wr_E_file -o $wr_cutnorm_outfile -e $CSDP_exe"
$PYTHON CutnormApprox.py -m $zfile -s $wr_E_file -o $wr_cutnorm_outfile -e $CSDP_exe &
pid_1=$!
#echo "CutNorm computed! ($wr_cutnorm_outfile)"

#echo "$PYTHON CutnormApprox.py -m $zfile -s $fn_E_file -o $fn_cutnorm_outfile -e $CSDP_exe"
$PYTHON CutnormApprox.py -m $zfile -s $fn_E_file -o $fn_cutnorm_outfile -e $CSDP_exe &
pid_2=$!
#echo "CutNorm computed! ($fn_cutnorm_outfile)"

#echo "$PYTHON CutnormApprox.py -m $zfile -s $cf_E_file -o $cf_cutnorm_outfile -e $CSDP_exe"
$PYTHON CutnormApprox.py -m $zfile -s $cf_E_file -o $cf_cutnorm_outfile -e $CSDP_exe &
pid_3=$!
#echo "CutNorm computed! ($cf_cutnorm_outfile)"

#echo "$PYTHON CutnormApprox.py -m $zfile -s $di_E_file -o $di_cutnorm_outfile -e $CSDP_exe"
$PYTHON CutnormApprox.py -m $zfile -s $di_E_file -o $di_cutnorm_outfile -e $CSDP_exe &
pid_4=$!
#echo "CutNorm computed! ($di_cutnorm_outfile)"


z_cutnorm_outfile=$zfile.cutnorm.csv
#echo "Computing Z-cut norm .."
$PYTHON CutnormApprox.py -m $zfile -o $z_cutnorm_outfile -e $CSDP_exe
#echo "Cutnorm for Z computed!"

wait $pid_1
wait $pid_2
wait $pid_3
wait $pid_4

#echo "processing cutnorm results .."
COUNT=0
wr_bound=''
while read -r line; do
	COUNT=$(( $COUNT + 1))
	if [ $COUNT == 4 ]; then
		wr_bound=${line:-1}
	fi
done < $wr_cutnorm_outfile

COUNT=0
fn_bound=''
while read -r line; do
        COUNT=$(( $COUNT + 1))
        if [ $COUNT == 4 ]; then
                fn_bound=${line:-1}
        fi
done < $fn_cutnorm_outfile

COUNT=0
cf_bound=''
while read -r line; do
        COUNT=$(( $COUNT + 1))
        if [ $COUNT == 4 ]; then
                cf_bound=${line:-1}
        fi
done < $cf_cutnorm_outfile

COUNT=0
di_bound=''
while read -r line; do
        COUNT=$(( $COUNT + 1))
        if [ $COUNT == 4 ]; then
                di_bound=${line:-1}
        fi
done < $di_cutnorm_outfile


COUNT=0
z_bound=''
while read -r line; do
        COUNT=$(( $COUNT + 1))
        if [ $COUNT == 4 ]; then
                z_bound=${line:-1}
        fi
	if [ $COUNT == 10 ];then
		IFS=',' read -r -a earr <<< "$line"
		eval_count=0
		for i in "${earr[@]}"; do
			k=${#narr[@]}
			for j in "${narr[@]}";do
				if [ $i == $j ]; then
					eval_count=$(($eval_count + 1))
				fi
			done
			if [ $eval_count == 0 ];then
				narr[$k]=$i
			fi
		done
		# Cut norm estimate of Z-A: [a,b]
                # Cut norm estimate of Z: [c,d]		
		IFS=',' read -r -a zarr <<< "$z_bound"
                z_lb=${zarr[0]:1}       
                z_ub=${zarr[1]}
		zl=${#z_ub}
		zl=$(($zl - 1))
		z_ub=${z_ub:0:$zl}

		IFS=',' read -r -a warr <<< "$wr_bound"
                wr_lb=${warr[0]:1}
                wr_ub=${warr[1]}
		wl=${#wr_ub}
                wl=$(($wl - 1))
                wr_ub=${wr_ub:0:$wl}

                IFS=',' read -r -a farr <<< "$fn_bound"
                fn_lb=${farr[0]:1}
                fn_ub=${farr[1]}
		fl=${#fn_ub}
                fl=$(($fl - 1))
                fn_ub=${fn_ub:0:$fl}

		IFS=',' read -r -a cfarr <<< "$cf_bound"
                cf_lb=${cfarr[0]:1}
                cf_ub=${cfarr[1]}
                cfl=${#cf_ub}
                cfl=$(($cfl - 1))
                cf_ub=${cf_ub:0:$cfl}

		IFS=',' read -r -a darr <<< "$di_bound"
                di_lb=${darr[0]:1}
                di_ub=${darr[1]}
                dil=${#di_ub}
                dil=$(($dil - 1))
                di_ub=${di_ub:0:$dil}

                # ||wr_bound-Z|| = [a/normVal,b/normVal]
                #wr_nlb=$(($wr_lb / $normVal))
                #wr_nub=$(($wr_ub / $normVal))

		if [ $k == 2 ];then
			# if eigenValue has exactly 2 distinct values then 
			# Normalized cut norm estimate of Z-A: [a/c, b/c]
			normVal=$z_lb	
			wr_nlb=$(echo "scale=4; $wr_lb/$normVal" | bc)
			wr_nub=$(echo "scale=4; $wr_ub/$normVal" | bc)

			fn_nlb=$(echo "scale=4; $fn_lb/$normVal" | bc)
			fn_nub=$(echo "scale=4; $fn_ub/$normVal" | bc)

                        cf_nlb=$(echo "scale=4; $cf_lb/$normVal" | bc)
                        cf_nub=$(echo "scale=4; $cf_ub/$normVal" | bc)

                        di_nlb=$(echo "scale=4; $di_lb/$normVal" | bc)
                        di_nub=$(echo "scale=4; $di_ub/$normVal" | bc)


		fi
		if [ $k -ne 2 ];then
			# Normalized cut norm estimate of Z-A: [a/d, b/c]
			wr_nlb=$(echo "scale=4; $wr_lb/$z_ub" | bc)
                        wr_nub=$(echo "scale=4; $wr_ub/$z_lb" | bc)

                        fn_nlb=$(echo "scale=4; $fn_lb/$z_ub" | bc)
                        fn_nub=$(echo "scale=4; $fn_ub/$z_lb" | bc)

                        cf_nlb=$(echo "scale=4; $cf_lb/$z_ub" | bc)
                        cf_nub=$(echo "scale=4; $cf_ub/$z_lb" | bc)

			di_nlb=$(echo "scale=4; $di_lb/$z_ub" | bc)
                        di_nub=$(echo "scale=4; $di_ub/$z_lb" | bc)

		fi
	fi
done < $z_cutnorm_outfile

statout_dir=cutnorm_results
if [ ! -d $statout_dir ];then
	mkdir $statout_dir
fi
statout=$statout_dir/$graphname.cutnorms.csv
rm -f $statout

echo -e "graphName\tno_samples\tWR_low_cutnorm,WR_up_cutnorm\tFN_low_cutnorm,FN_up_cutnorm\tcomoF_low_cutnorm,comoF_up_cutnorm\tdiamcis_low_cutnorm,diamcis_up_cutnorm\tZ_low_cutnorm,Z_up_cutnorm\tWR_low_norm_cutnorm\tWR_up_norm_cutnorm\tFN_low_norm_cutnorm\tFN_up_norm_cutnorm\tcomoF_low_norm_cutnorm\tcomoF_up_norm_cutnorm\tdiamcis_low_norm_cutnorm\tdiamcis_up_norm_cutnorm" >> $statout

echo -e "$graphname\t$ngraphs\t$wr_bound\t$fn_bound\t$cf_bound\t$di_bound\t$z_bound\t$wr_nlb\t$wr_nub\t$fn_nlb\t$fn_nub\t$cf_nlb\t$cf_nub\t$di_nlb\t$di_nub" >> $statout



echo "Program finished successfully and results saved into $statout file!"
