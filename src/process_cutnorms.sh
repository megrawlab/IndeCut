#!/bin/bash

wr_cutnorm_outfile=$1
fn_cutnorm_outfile=$2
cf_cutnorm_outfile=$3
di_cutnorm_outfile=$4
z_cutnorm_outfile=$5
result_outfile=$6
graphname=$7
ngraphs=$8

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

if [ -f $di_cutnorm_outfile ]; then
COUNT=0
di_bound=''
while read -r line; do
        COUNT=$(( $COUNT + 1))
        if [ $COUNT == 4 ]; then
                di_bound=${line:-1}
        fi
done < $di_cutnorm_outfile
fi


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

		if [[ ! -z "${di_bound// }" ]]; then
		IFS=',' read -r -a darr <<< "$di_bound"
                di_lb=${darr[0]:1}
                di_ub=${darr[1]}
                dil=${#di_ub}
                dil=$(($dil - 1))
                di_ub=${di_ub:0:$dil}
		fi

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


#statout_dir=cutnorm_results
#if [ ! -d $statout_dir ];then
#	mkdir $statout_dir
#fi
#statout=$statout_dir/$graphname.cutnorms.csv
rm -f $result_outfile

echo -e "graphName\tno_samples\tWR_low_cutnorm,WR_up_cutnorm\tFN_low_cutnorm,FN_up_cutnorm\tcomoF_low_cutnorm,comoF_up_cutnorm\tdiamcis_low_cutnorm,diamcis_up_cutnorm\tZ_low_cutnorm,Z_up_cutnorm\tWR_low_norm_cutnorm\tWR_up_norm_cutnorm\tFN_low_norm_cutnorm\tFN_up_norm_cutnorm\tcomoF_low_norm_cutnorm\tcomoF_up_norm_cutnorm\tdiamcis_low_norm_cutnorm\tdiamcis_up_norm_cutnorm" >> $result_outfile

echo -e "$graphname\t$ngraphs\t$wr_bound\t$fn_bound\t$cf_bound\t$di_bound\t$z_bound\t$wr_nlb\t$wr_nub\t$fn_nlb\t$fn_nub\t$cf_nlb\t$cf_nub\t$di_nlb\t$di_nub" >> $result_outfile



echo "Program finished successfully and results saved into $result_outfile file!"

