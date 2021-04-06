#! /usr/bin/env sh

LOG_DIR=$1
DATASET=$2
num_patts=$3
OUT_DIR=$4

mkdir -pv $OUT_DIR
mkdir -pv $OUT_DIR/figs

nthreads=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 1 -d "-" | sort -n | uniq | tr "\n" " "`
ntours=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 2 -d "-" | sort -n | uniq | tr "\n" " "`
ks=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 3 -d "-" | sort -n | uniq | tr "\n" " "`

echo "# threads: $nthreads"
echo "# tours: $ntours"
echo "k's: $ks"

:'for t in $nthreads
do
        for k in $ks
        do
                for n in $ntours
                do
			pattfile="${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.patts"
			echo "patts: $pattfile"
			grep  "==>update final pattern " ${LOG_DIR}/matryoshka-${DATASET}-$t-$n-$k-*.out | cut -f 4,7 -d " " | sort -k1 -r -t " " | awk -f groupby-sum.awk | sort -k2 -r -t " "  > $pattfile

		done
	done
done'

for t in $nthreads
do
	for k in $ks
	do
		: '
		pattfile="${OUT_DIR}/matryoshka-${DATASET}-patts-$k.txt"
		echo "patts: $pattfile"
		grep  "==>update final pattern " ${LOG_DIR}/matryoshka-${DATASET}-$t-*-$k-*.out | cut -f 4,7 -d " " | sort -k2 -r -n | cut -f 1 -d " " | uniq > $pattfile
		echo "grep  \"==>update final pattern \" ${LOG_DIR}/matryoshka-${DATASET}-$t-*-$k-*.out | cut -f 4,7 -d \" \" | sort -k2 -r -n | cut -f 1 -d \" \" | uniq > $pattfile"
		for p in `head -n 10 $pattfile`
		do
			pastefiles=""
			header=`echo "$ntours" | tr "\n" "\t"`

			for n in $ntours
			do
				numExp=`ls -1 ${LOG_DIR}/matryoshka-${DATASET}-$t-$n-$k-*.out | wc -l`
				echo "ls -1 ${LOG_DIR}/matryoshka-${DATASET}-$t-$n-$k-*.out"
				outfile="${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k-$p.dat"
				echo "checking $outfile"
				grep "==>update final pattern $p " ${LOG_DIR}/matryoshka-${DATASET}-$t-$n-$k-*.out | cut -f 7 -d " " > $outfile
				found=`wc -l $outfile | cut -f 1 -d " "`
				remain=`echo "$numExp - $found" | bc -l`
				echo "numExp $numExp found $found remain $remain"
				for i in `seq 1 $remain` 
				do
					echo "0" >> $outfile	
				done
				pastefiles="$pastefiles $outfile"
			done

			#concate all tours in the order of ntours
			paste $pastefiles > ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.dat
			echo "paste $pastefiles > ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.dat"

			gnuplot -e "inputname='${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.dat'" -e "outputname='${OUT_DIR}/figs/matryoshka-${DATASET}-$t-$k-$p-boxplot.eps'" -e "header='$header'" plot-boxplot.gp

			#get mean and stdesv
			awk -f get-mean-stdesv.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.dat | awk -f transpose.awk > ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.stat
			#insert header
			#sed -i "1s/^/$header\n/" ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.stat
			#awk -f transpose.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.stat | 
			gnuplot -e "inputname='${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.stat'" -e "outputname='${OUT_DIR}/figs/matryoshka-${DATASET}-$t-$k-$p-avg-stdesv.eps'" -e "header='$header'" plot-avg-stddesv.gp

			#rm $pastefiles
		done
		'
		#system issues
		pastefiles=""
		for n in $ntours
		do
			#Elapsed (wall clock) time (h:mm:ss or m:ss):
			#Maximum resident set size (kbytes):
			#File system outputs:
			grep "Elapsed (wall clock) time (h:mm:ss or m:ss): " ${LOG_DIR}/matryoshka-${DATASET}-$t-$n-$k-*.out | cut -f 8 -d " " | tr ":" " " | awk -f get-timesec.awk | awk -v value=3600 -f div-cols-by.awk > ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.time
			awk -f get-mean-stdesv.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.time | awk -f transpose.awk >> ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.time
			
			grep "Maximum resident set size (kbytes): " ${LOG_DIR}/matryoshka-${DATASET}-$t-$n-$k-*.out | cut -f 6 -d " " | awk -v value=1048576 -f div-cols-by.awk > ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.mem
			awk -f get-mean-stdesv.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.mem | awk -f transpose.awk >> ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.mem
			
			grep "File system outputs: " ${LOG_DIR}/matryoshka-${DATASET}-$t-$n-$k-*.out | cut -f 4 -d " " | awk -v value=1048576 -f div-cols-by.awk > ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.disc
			awk -f get-mean-stdesv.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.disc | awk -f transpose.awk >> ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.disc
			
		done
		paste ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.time ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.mem ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.disc -d "c"  | sed "s/ / $\\\pm$ /g" | sed "s/c/ \& /g" | sed "s/$/ \\/g"> ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.table.sys
	done
done 	
