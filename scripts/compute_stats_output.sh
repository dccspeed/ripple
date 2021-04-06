#! /usr/bin/env sh

OUTPUT_DIR=$1
TRUE_VALUE=$2

ns=`ls -1 ${OUTPUT_DIR} | cut -f 4 -d "_" | sed "s/\.txt//g" | sort -n | uniq | tr "\n" " "`
echo "$ns"
for i in `echo $ns`;
do
		echo "#################"
		echo "=>number of tours $i<="
		n=`tail -n 10 ${OUTPUT_DIR}/output_*_config_${i}.txt | grep "total count estimate:" | wc -l`
		#echo "tail -n 10 ${OUTPUT_DIR}/output_*_config_${i}.txt | grep \"total count estimate:\" | wc -l"
		echo "n $n"
		total=`tail -n 10 ${OUTPUT_DIR}/output_*_config_${i}.txt | grep "total count estimate:" | cut -f 4 -d " " | paste -sd+ | bc -l`
		echo "total $total"
		mean=`echo "$total / $n" | bc -l`
		echo "mean $mean"
	
		var=0
		vartrue=0
		for j  in `tail -n 10 ${OUTPUT_DIR}/output_*_config_${i}.txt | grep "total count estimate:" | cut -f 4 -d " " | tr "\n" " "`
		do
			#echo "$j"
			
			#with mean value
			x=`echo "$mean - $j" | bc -l`
			x=`echo "$x ^ 2" | bc -l`
			#echo "$x" 
			var=`echo "$var + $x" | bc -l`
			
			#with true value
			xtrue=`echo "$TRUE_VALUE - $j" | bc -l`
			xtrue=`echo "$xtrue ^ 2" | bc -l`
			#echo "$xtrue" 
			vartrue=`echo "$vartrue + $xtrue" | bc -l`
			#echo "$vartrue + $xtrue | bc -l"
		done

		var=`echo "$var / ($n - 1)" | bc -l`
		sd=`echo "sqrt($var)" | bc -l`
		echo "var $var"
		echo "sd $sd"
		
		vartrue=`echo "$vartrue / $n" | bc -l`
		sdtrue=`echo "sqrt($vartrue)" | bc -l`
		echo "vartrue $vartrue"
		echo "sdtrue $sdtrue"
	
		echo "#################"
done
