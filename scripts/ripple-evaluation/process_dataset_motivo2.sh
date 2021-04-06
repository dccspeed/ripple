#!/usr/bin/env bash

LOG_DIR=$1
DATASET=$2
OUT_DIR=$3
MAX_PATTS=$4

mkdir -pv $OUT_DIR
mkdir -pv $OUT_DIR/figs

nthreads=`ls ${LOG_DIR}/motivo-${DATASET}*out2 | tr " " "\n" | sed "s/.*mtv-//g" | sed "s/\.out2//g" | cut -f 1 -d "-" | sort -n | uniq | tr "\n" " "`
ks=`ls ${LOG_DIR}/motivo-${DATASET}*out2 | tr " " "\n" | sed "s/.*mtv-//g" | sed "s/\.out2//g" | cut -f 2 -d "-" | sort -n | uniq | tr "\n" " "`

echo "# threads: $nthreads"
echo "k's: $ks"

echo "getting the top most frequent subgraphs"
for t in $nthreads
do
        for k in $ks
	do
		num_ids=`ls -1 ${LOG_DIR}/motivo-${DATASET}-$t-$k-*.out2 | wc -l`
		echo "number of executions $num_ids"
		pattfile="${OUT_DIR}/motivo-${DATASET}-$t-$k.patts"
		echo "pattfile: $pattfile"
		grep  "==>update final pattern " ${LOG_DIR}/motivo-${DATASET}-$t-$k-*.out2 | cut -f 4,7 -d " " | sort -k1 -n -r -t " " | awk -f groupby-sum.awk | awk -v num_ids=${num_ids} '{ printf("%d %lf\n", $1, $2/num_ids) }' | sort -k2 -r -g  -t " "  > $pattfile

	done
done


CSV_FILE="${OUT_DIR}/motivo-${DATASET}-nogt.csv"
echo "graph threads k id pattern_id pattern_id_inc estimate avg desv relative_desv" > ${CSV_FILE}

for bifile in `ls -1 ${LOG_DIR}/motivo-${DATASET}-*.out2`
do
	echo "checking file: $bifile"
	graph="$(basename $bifile | sed 's/motivo-\(.*\).mtv-\(.*\)-\(.*\)-\(.*\).out2/\1/g')"
	t="$(basename $bifile | sed 's/motivo-\(.*\).mtv-\(.*\)-\(.*\)-\(.*\).out2/\2/g')"
	k="$(basename $bifile | sed 's/motivo-\(.*\).mtv-\(.*\)-\(.*\)-\(.*\).out2/\3/g')"
	id="$(basename $bifile | sed 's/motivo-\(.*\).mtv-\(.*\)-\(.*\)-\(.*\).out2/\4/g')"

	#echo "graph=$graph"
	#echo "threads=$threads"
	#echo "size=$size"
	#echo "iteration_id=$iteration_id"

	outfile="${OUT_DIR}/motivo-${DATASET}-$t-$k-$id.cnt"
	pattfreqfile="${OUT_DIR}/motivo-${DATASET}-$t-$k.patts"
	#pattcountfile="${OUT_DIR}/motivo-${DATASET}-$t-$k.patts"
	pattcountfile=${pattfreqfile}

	#get the patts from the most accurate evaluation (larger number of tours)
	patts=`head -n $MAX_PATTS ${pattfreqfile} | cut -f 1 -d " " | tr "\n" " "`
	#echo "patts: $patts"

	echo "top frequent pattern file $pattfreqfile"
	echo "pattern count file: $pattcountfile"
	echo "output file: $outfile"

	pinc=1
	for p in $patts
	do
		#echo "pattern: $p"
		gt=$(grep "^$p " $pattcountfile | cut -f 2 -d " ")
		if [ -z "$gt" ]; then
			echo "is gt empty"
			gt="0"
		fi

		count=$(grep "==>update final pattern $p " $bifile | cut -f 7 -d " ")
		if [ -z "$count" ]; then
			echo "is count empty"
			count="0"
		fi

		error=0
		if [ $gt != "0" ] && [ $count != "0" ]; then
			error=`echo "sqrt(($gt-$count)*($gt-$count))" | bc -l`
		fi

		rlerror=0
		if [ $gt != "0" ]; then
			rlerror=`echo "$error / $gt" | bc -l`
		fi

		#echo "patt $p gt $gt count $count error $error"
		echo "$p $count $gt $error $rlerror" >> $outfile
		echo "$graph $t $k $id $p $pinc $count $gt $error $rlerror" >> ${CSV_FILE}
		#pinc=`echo "$pinc + 1" | bc`
		pinc=$((pinc+1))
	done 

done

exit

:'
nthreads=`ls -1 ${OUT_DIR}/motivo-${DATASET}*.cnt | sed "s/.*gph-//g" | sed "s/\.cnt//g" | cut -f 1 -d "-" | sort -n | uniq | tr "\n" " "`
ntours=`ls -1 ${OUT_DIR}/motivo-${DATASET}*.cnt | sed "s/.*gph-//g" | sed "s/\.cnt//g" | cut -f 2 -d "-" | sort -n | uniq | tr "\n" " "`
ks=`ls -1 ${OUT_DIR}/motivo-${DATASET}*.cnt | sed "s/.*gph-//g" | sed "s/\.cnt//g" | cut -f 3 -d "-" | sort -n | uniq | tr "\n" " "`

echo "# threads: $nthreads"
echo "# tours: $ntours"
echo "ks: $ks"

for t in $nthreads
do
	for k in $ks
	do
		pastefiles=""
		header=`echo "$ntours" | tr "\n" "\t"`
		for n in $ntours
		do
			ids=`ls -1 ${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-*.cnt | sed "s/.*gph-//g" | sed "s/\.cnt//g" | cut -f 4 -d "-" | sort -n | uniq | tr "\n" " "`
			#echo "ids: $ids"	
			outfile="${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-cnt.dat"
			pastefiles="$pastefiles $outfile"
			pasteerrorfiles=""
			for id in $ids
			do
				pasteerrorfiles="$pasteerrorfiles <(cut -f 5 -d \" \" ${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-$id.cnt)"
				infile="${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-$id.cnt"
				terror=`cut -f 5 -d " " $infile | paste -sd+ | bc -l`
				#echo "cut -f 4 -d " " $infile | paste -sd+ | bc -l"
				#echo "terror $terror in $infile"
				echo "$terror" >> $outfile
			done

			patts=`cut -f 1 -d " " ${ESCAPE_DIR}/${ESCAPE_FILE}-$k.out.counts | tr "\n" " "`
			num_patts=`wc -l ${ESCAPE_DIR}/${ESCAPE_FILE}-$k.out.counts | cut -f 1 -d " "`
			patts_inc=`seq -s ' ' 1 $num_patts`
			#echo "$patts_inc"
			#echo "patts: $patts"
			#echo "paste $pasteerrorfiles >  ${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-cnt-error.dat"
			eval "paste $pasteerrorfiles | awk -f transpose.awk >  ${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-cnt-error.dat"

			#awk -f transpose.awk ${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-cnt-error.dat 
			#echo "awk -f transpose.awk ${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-cnt-error.dat"
			gnuplot -e "inputname='${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-cnt-error.dat'" -e "outputname='${OUT_DIR}/figs/motivo-${DATASET}-$t-$n-$k-cnt-error.eps'" -e "header='$patts_inc'" -e "labelx='Subgraph Pattern Ids'"  -e "labely='Relative Error'" plot-boxplot.gp
			awk -f get-mean-stdesv.awk ${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-cnt-error.dat | awk -f transpose.awk > ${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-cnt-error-avg-sdesv.stat
			gnuplot -e "inputname='${OUT_DIR}/motivo-${DATASET}-$t-$n-$k-cnt-error-avg-sdesv.stat'" -e "outputname='${OUT_DIR}/figs/motivo-${DATASET}-$t-$n-$k-cnt-error-avg-stdesv.eps'" -e "header='$patts_inc'" -e "labelx='Subgraph Pattern Ids'"  -e "labely='Relative Error'" plot-avg-stddesv.gp

		done
		paste $pastefiles > ${OUT_DIR}/motivo-${DATASET}-$t-$k-cnt.dat
		echo "output final file: ${OUT_DIR}/motivo-${DATASET}-$t-$k-cnt.dat"
		awk -f get-mean-stdesv.awk ${OUT_DIR}/motivo-${DATASET}-$t-$k-cnt.dat | awk -f transpose.awk > ${OUT_DIR}/motivo-${DATASET}-$t-$k-cnt.stat
		#insert header
		#sed -i "1s/^/$header\n/" ${OUT_DIR}/motivo-${DATASET}-$t-$k-cnt.stat
		gnuplot -e "inputname='${OUT_DIR}/motivo-${DATASET}-$t-$k-cnt.stat'" -e "outputname='${OUT_DIR}/figs/motivo-${DATASET}-$t-$k-cnt-avg-stdesv.eps'" -e "header='$header'" -e "labelx='Number of tours'"  -e "labely='Total Relative Error'" plot-avg-stddesv.gp

		echo "gnuplot -e \"inputname='${OUT_DIR}/motivo-${DATASET}-$t-$k-cnt.stat'\" -e \"outputname='${OUT_DIR}/figs/motivo-${DATASET}-$t-$k-cnt-avg-stdesv.eps'\" -e \"header='$header'\" plot-avg-stddesv.gp"
	done
done
'

