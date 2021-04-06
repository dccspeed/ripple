#! /usr/bin/env sh

LOG_DIR=$1
DATASET=$2
OUT_DIR=$3

mkdir -pv $OUT_DIR
mkdir -pv $OUT_DIR/figs

nthreads=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 1 -d "-" | sort -n | uniq | tr "\n" " "`
ntours=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 2 -d "-" | sort -n | uniq | tr "\n" " "`
ks=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 3 -d "-" | sort -n | uniq | tr "\n" " "`

echo "# threads: $nthreads"
echo "# tours: $ntours"
echo "k's: $ks"

CSV_FILE="${OUT_DIR}/matryoshka-${DATASET}-sys.csv"
echo "graph threads tours k id time memory" > ${CSV_FILE}
for bifile in `ls -1 ${LOG_DIR}/matryoshka-${DATASET}-*.out`
do
	echo "checking file: $bifile"
	graph="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\1/g')"
        t="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\2/g')"
        n="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\3/g')"
        k="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\4/g')"
        id="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\5/g')"

	#Elapsed (wall clock) time (h:mm:ss or m:ss):
	tm=`grep "Elapsed (wall clock) time (h:mm:ss or m:ss): " ${bifile} | cut -f 8 -d " " | tr ":" " " | awk -f get-timesec.awk | awk -v value=3600 -f div-cols-by.awk`
	#echo "grep \"Elapsed (wall clock) time (h:mm:ss or m:ss): \" ${bifile} | cut -f 8 -d \" \" | tr ':' ' ' | awk -f get-timesec.awk | awk -v value=3600 -f div-cols-by.awk"
	echo "$tm" >> ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.time

	#Maximum resident set size (kbytes):
	ms=`grep "Maximum resident set size (kbytes): " ${bifile} | cut -f 6 -d " " | awk -v value=1048576 -f div-cols-by.awk` 
	echo "$ms" >> ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.mem

	echo "$graph $t $n $k $id $tm $ms" >> $CSV_FILE
	#File system outputs: 
	#grep "File system outputs: " ${LOG_DIR}/matryoshka-${DATASET}-$t-$n-$k-$id.out | cut -f 4 -d " " | awk -v value=1048576 -f div-cols-by.awk > ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.disc
done 	
	

CSV_FILE2="${OUT_DIR}/matryoshka-${DATASET}-sys-avg-sdesv.csv"
echo "graph threads tours k timeavg timesdesv memavg memsdesv" > ${CSV_FILE2}
for t in $nthreads
do
	for k in $ks
	do
		for n in $ntours
		do
			
			stat=`awk -f get-mean-stdesv.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.time | awk -f transpose.awk`
		       	echo "$stat" >> ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.time
			tm_avg=`echo "$stat" | cut -f 1 -d " "` 
			tm_stdesv=`echo "$stat" | cut -f 2 -d " "` 
			
			stat=`awk -f get-mean-stdesv.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.mem | awk -f transpose.awk`
		        echo "$stat" >> ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.mem
			mem_avg=`echo "$stat" | cut -f 1 -d " "` 
			mem_stdesv=`echo "$stat" | cut -f 2 -d " "` 
			
			#awk -f get-mean-stdesv.awk ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-$k.disc | awk -f transpose.awk >> ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.disc
		        echo "$graph $t $n $k $tm_avg $tm_stdesv $mem_avg $mem_stdesv" >> ${CSV_FILE2}	

		done
	done
done

#paste ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.time ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.mem -d "c"  | sed "s/ / $\\\pm$ /g" | sed "s/c/ \& /g" > ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.table.sys
#paste ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.time ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.mem ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.disc -d "c"  | sed "s/ / $\\\pm$ /g" | sed "s/c/ \& /g" > ${OUT_DIR}/matryoshka-${DATASET}-$t-$n-avg-stdesv.table.sys
