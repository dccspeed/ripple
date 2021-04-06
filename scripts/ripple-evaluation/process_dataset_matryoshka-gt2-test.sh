#!/usr/bin/env bash

LOG_DIR=$1
DATASET=$2
ESCAPE_DIR=$3
ESCAPE_FILE=$4
OUT_DIR=$5

mkdir -pv $OUT_DIR
mkdir -pv $OUT_DIR/figs

#nthreads=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 1 -d "-" | sort -n | uniq | tr "\n" " "`
#ntours=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 2 -d "-" | sort -n | uniq | tr "\n" " "`
#ks=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 3 -d "-" | sort -n | uniq | tr "\n" " "`

#echo "# threads: $nthreads"
#echo "# tours: $ntours"
#echo "k's: $ks"

#for bifile in `ls -1 ${LOG_DIR}/matryoshka-${DATASET}-40-1000-3-*.out`
for bifile in `ls -1 ${LOG_DIR}/matryoshka-${DATASET}-*.out`
do
	echo "checking file: $bifile"
	graph="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\1/g')"
        t="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\2/g')"
        n="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\3/g')"
        k="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\4/g')"
        id="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\5/g')"

        #echo "graph=$graph"
        #echo "threads=$threads"
        #echo "ntours=$ntours"
        #echo "size=$size"
        #echo "iteration_id=$iteration_id"

	patts=`cut -f 1 -d " " ${ESCAPE_DIR}/${ESCAPE_FILE}-$k.out.counts`
	outfile="${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-$id.cnt"
	escapefile="${ESCAPE_DIR}/${ESCAPE_FILE}-$k.out.counts"
	echo "checking escape file: $escapefile"
	echo "output file: $outfile"

	while IFS= read -r line; do
		#echo "line $line"
		p=`echo "$line" | cut -f 1 -d " "`
		gt=`echo "$line" | cut -f 2 -d " "`
		count=$(grep "==>update final pattern $p " $bifile | cut -f 7 -d " ")
		#echo "grep \"==>update final pattern $p \" $bifile | cut -f 7 -d \" \""
		if [ -z "$count" ]; then
			echo "is empty $count"
			count=0
		fi
		error=`echo "sqrt(($gt-$count)*($gt-$count))" | bc -l`
		rlerror=`echo "$error / $gt" | bc -l`
		#echo "patt $p gt $gt count $count error $error"
		echo "$p $count $gt $error $rlerror" >> $outfile
	done < "${ESCAPE_DIR}/${ESCAPE_FILE}-$k.out.counts"

done

nthreads=`ls -1 ${OUT_DIR}/matryoshka-${DATASET}*.cnt | sed "s/.*gph-//g" | sed "s/\.cnt//g" | cut -f 1 -d "-" | sort -n | uniq | tr "\n" " "`
ntours=`ls -1 ${OUT_DIR}/matryoshka-${DATASET}*.cnt | sed "s/.*gph-//g" | sed "s/\.cnt//g" | cut -f 2 -d "-" | sort -n | uniq | tr "\n" " "`
ks=`ls -1 ${OUT_DIR}/matryoshka-${DATASET}*.cnt | sed "s/.*gph-//g" | sed "s/\.cnt//g" | cut -f 3 -d "-" | sort -n | uniq | tr "\n" " "`

echo "# threads: $nthreads"
echo "# tours: $ntours"
echo "k's: $ks"

for t in $nthreads
do
	for k in $ks
	do
		pastefiles=""
		header=`echo "$ntours" | tr "\n" "\t"`
		for n in $ntours
		do
			ids=`ls -1 ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-*.cnt | sed "s/.*gph-//g" | sed "s/\.cnt//g" | cut -f 4 -d "-" | sort -n | uniq | tr "\n" " "`
			#echo "ids: $ids"	
			outfile="${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt.dat"
			pastefiles="$pastefiles $outfile"
			pasteerrorfiles=""
			for id in $ids
			do
				pasteerrorfiles="$pasteerrorfiles <(cut -f 5 -d \" \" ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-$id.cnt)"
				infile="${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-$id.cnt"
				terror=`cut -f 4 -d " " $infile | paste -sd+ | bc -l`
				#echo "cut -f 4 -d " " $infile | paste -sd+ | bc -l"
				#echo "terror $terror in $infile"
				echo "$terror" >> $outfile
			done
	
			patts=`cut -f 1 -d " " ${ESCAPE_DIR}/${ESCAPE_FILE}-$k.out.counts | tr "\n" " "`
			#echo "patts: $patts"
			rm -f ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-avg-sdesv.tmp
			for p in $patts; do
				cat ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.stat >> ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-avg-sdesv.tmp
			done

			#echo "paste $pasteerrorfiles >  ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error.dat"
			eval "paste $pasteerrorfiles | awk -f transpose.awk >  ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error.dat"
			echo "-------=========================="
			cat ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error.dat
			echo "-------=========================="

			#awk -f transpose.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error.dat 
			#echo "awk -f transpose.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error.dat"
			gnuplot -e "inputname='${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error.dat'" -e "outputname='${OUT_DIR}/figs/matryoshka-${DATASET}-$t-$n-$k-cnt-error.eps'" -e "header='$patts'" plot-boxplot.gp

			awk -f get-mean-stdesv.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error.dat | awk -f transpose.awk > ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error-avg-sdesv.stat.tmp
			paste -d ' ' ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error-avg-sdesv.stat.tmp ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-avg-sdesv.tmp | sort -n -r -k3,3 > ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error-avg-sdesv.stat
			rm -f ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error-avg-sdesv.stat.tmp
			rm -f ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-avg-sdesv.tmp

			gnuplot -e "inputname='${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-cnt-error-avg-sdesv.stat'" -e "outputname='${OUT_DIR}/figs/matryoshka-${DATASET}-$t-$n-$k-cnt-error-avg-stdesv.eps'" -e "header='$patts'" plot-avg-stddesv-test.gp

		done
	       	paste $pastefiles > ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-cnt.dat
		echo "output final file: ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-cnt.dat"
		awk -f get-mean-stdesv.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-cnt.dat | awk -f transpose.awk > ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-cnt.stat
		#insert header
		#sed -i "1s/^/$header\n/" ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-cnt.stat
		gnuplot -e "inputname='${OUT_DIR}/matryoshka-${DATASET}-$t-$k-cnt.stat'" -e "outputname='${OUT_DIR}/figs/matryoshka-${DATASET}-$t-$k-cnt-avg-stdesv.eps'" -e "header='$header'" plot-avg-stddesv.gp
				
		echo "gnuplot -e \"inputname='${OUT_DIR}/matryoshka-${DATASET}-$t-$k-cnt.stat'\" -e \"outputname='${OUT_DIR}/figs/matryoshka-${DATASET}-$t-$k-cnt-avg-stdesv.eps'\" -e \"header='$header'\" plot-avg-stddesv.gp"
	done
done

