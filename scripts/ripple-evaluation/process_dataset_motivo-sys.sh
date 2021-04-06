#! /usr/bin/env sh

LOG_DIR=$1
DATASET=$2
OUT_DIR=$3

mkdir -pv $OUT_DIR
mkdir -pv $OUT_DIR/figs

nthreads=`ls ${LOG_DIR}/motivo-${DATASET}*out | tr " " "\n" | sed "s/.*mtv-//g" | sed "s/\.out//g" | cut -f 1 -d "-" | sort -n | uniq | tr "\n" " "`
ks=`ls ${LOG_DIR}/motivo-${DATASET}*out | tr " " "\n" | sed "s/.*mtv-//g" | sed "s/\.out//g" | cut -f 2 -d "-" | sort -n | uniq | tr "\n" " "`

echo "# threads: $nthreads"
echo "k's: $ks"


CSV_FILE="${OUT_DIR}/motivo-${DATASET}-sys.csv"
echo "graph threads k id time memory" > ${CSV_FILE}
for bifile in `ls -1 ${LOG_DIR}/motivo-${DATASET}-*.out`
do
	echo "checking file: $bifile"
	graph="$(basename $bifile | sed 's/motivo-\(.*\).mtv-\(.*\)-\(.*\)-\(.*\).out/\1/g')"
        t="$(basename $bifile | sed 's/motivo-\(.*\).mtv-\(.*\)-\(.*\)-\(.*\).out/\2/g')"
        k="$(basename $bifile | sed 's/motivo-\(.*\).mtv-\(.*\)-\(.*\)-\(.*\).out/\3/g')"
        id="$(basename $bifile | sed 's/motivo-\(.*\).mtv-\(.*\)-\(.*\)-\(.*\).out/\4/g')"

	tm=`grep "build_time" ${LOG_DIR}/motivo-${DATASET}-$t-$k-$id.out | sed "s/.*time = //g" | awk -v value=3600 -f div-cols-by.awk`
	echo "$tm" >> ${OUT_DIR}/motivo-${DATASET}-$t-$k.time
	ms=`grep "disk usage=" ${LOG_DIR}/motivo-${DATASET}-$t-$k-$id.out | cut -f 2 -d "=" |  awk -f convert-mem-scales.awk | awk -v value=1048576 -f div-cols-by.awk` 
	echo "$ms" >> ${OUT_DIR}/motivo-${DATASET}-$t-$k.mem
	
	echo "$graph $t $k $id $tm $ms" >> $CSV_FILE
done 	
	

CSV_FILE2="${OUT_DIR}/motivo-${DATASET}-sys-avg-sdesv.csv"
echo "graph threads k timeavg timesdesv memavg memsdesv" > ${CSV_FILE2}
for t in $nthreads
do
	for k in $ks
	do
		stat=`awk -f get-mean-stdesv.awk ${OUT_DIR}/motivo-${DATASET}-$t-$k.time | awk -f transpose.awk`
		echo "$stat" >> ${OUT_DIR}/motivo-${DATASET}-$t-avg-stdesv.time
		tm_avg=`echo "$stat" | cut -f 1 -d " "` 
		tm_stdesv=`echo "$stat" | cut -f 2 -d " "` 

		stat=`awk -f get-mean-stdesv.awk ${OUT_DIR}/motivo-${DATASET}-$t-$k.mem | awk -f transpose.awk`
		echo "$stat" >> ${OUT_DIR}/motivo-${DATASET}-$t-avg-stdesv.mem
		mem_avg=`echo "$stat" | cut -f 1 -d " "` 
		mem_stdesv=`echo "$stat" | cut -f 2 -d " "` 

		echo "$graph $t $k $tm_avg $tm_stdesv $mem_avg $mem_stdesv" >> ${CSV_FILE2}	
	done
done
