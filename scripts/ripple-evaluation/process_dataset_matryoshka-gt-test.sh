#! /usr/bin/env sh

LOG_DIR=$1
DATASET=$2
ESCAPE_DIR=$3
ESCAPE_FILE=$4
OUT_DIR=$5

mkdir -pv $OUT_DIR
mkdir -pv $OUT_DIR/figs

nthreads=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 1 -d "-" | sort -n | uniq | tr "\n" " "`
ntours=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 2 -d "-" | sort -n | uniq | tr "\n" " "`
ks=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 3 -d "-" | sort -n | uniq | tr "\n" " "`

echo "# threads: $nthreads"
echo "# tours: $ntours"
echo "k's: $ks"

for t in $nthreads
do
		for k in $ks
		do
			pattfile="${OUT_DIR}/matryoshka-${DATASET}-patts-$k.txt"
			grep  "==>update final pattern " ${LOG_DIR}/matryoshka-${DATASET}-$t-*-$k-*.out | cut -f 4 -d " " | sort -n | uniq > $pattfile
			echo "grep  \"==>update final pattern \" ${LOG_DIR}/matryoshka-${DATASET}-$t-*-$k-*.out | cut -f 4 -d \" \" | sort -n | uniq > $pattfile"
			for p in `cat $pattfile`
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
				truth=`grep $p ${ESCAPE_DIR}/${ESCAPE_FILE}-$k.out.counts | cut -f 2 -d " "` 
				gnuplot -e "inputname='${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.dat'" -e "outputname='${OUT_DIR}/figs/matryoshka-${DATASET}-$t-$k-$p-boxplot.eps'" -e "header='$header'" -e "truth=$truth" plot-boxplot-gt.gp
				
				#get mean and stdesv
				awk -f get-mean-stdesv.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.dat | awk -f transpose.awk  > ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.stat
				#insert header
				#sed -i "1s/^/$header\n/" ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.stat
				#awk -f transpose.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.stat | 
				gnuplot -e "inputname='${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.stat'" -e "outputname='${OUT_DIR}/figs/matryoshka-${DATASET}-$t-$k-$p-avg-stdesv.eps'" -e "header='$header'" -e "truth=$truth" plot-avg-stddesv-gt.gp
				
				if [ -n "$truth" ]; then
					awk -v gt=$truth -f get-errors.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p.dat > ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p-error.dat
					gnuplot -e "inputname='${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p-error.dat'" -e "outputname='${OUT_DIR}/figs/matryoshka-${DATASET}-$t-$k-$p-error-boxplot.eps'" -e "header='$header'" plot-boxplot.gp
					#get mean and stdesv
					awk -f get-mean-stdesv.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p-error.dat | awk -f transpose.awk > ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p-error.stat
					#insert header
					#sed -i "1s/^/$header\n/" ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p-error.stat
					#awk -f transpose.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p-error.stat | 
					gnuplot -e "inputname='${OUT_DIR}/matryoshka-${DATASET}-$t-$k-$p-error.stat'" -e "outputname='${OUT_DIR}/figs/matryoshka-${DATASET}-$t-$k-$p-error.eps'" -e "header='$header'" plot-avg-stddesv.gp
				fi
				echo "patt $p gt $truth"
				#rm $pastefiles
			done
		done
done 	
