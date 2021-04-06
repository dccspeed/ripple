#! /usr/bin/env sh

LOG_DIR=$1
DATASET=$2
OUT_DIR=$3

mkdir -pv $OUT_DIR

nthreads=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 1 -d "-" | sort -n | uniq | tr "\n" " "`
ntours=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 2 -d "-" | sort -n | uniq | tr "\n" " "`
ks=`ls ${LOG_DIR}/matryoshka-${DATASET}* | tr " " "\n" | sed "s/.*gph-//g" | sed "s/\.out//g" | cut -f 3 -d "-" | sort -n | uniq | tr "\n" " "`

echo "# threads: $nthreads"
echo "# tours: $ntours"
echo "k's: $ks"

CSV_FILE="${OUT_DIR}/matryoshka-${DATASET}-tour.csv"
rm -f $CSV_FILE

echo "graph threads tours k id partitionid partitionsize tourid tsteps subcount degreecount insertedrsv" > ${CSV_FILE}
for bifile in `ls -1 ${LOG_DIR}/matryoshka-${DATASET}-*.out`
do
	echo "checking file: $bifile"
	graph="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\1/g')"
        t="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\2/g')"
        n="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\3/g')"
        k="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\4/g')"
        id="$(basename $bifile | sed 's/matryoshka-\(.*\).gph-\(.*\)-\(.*\)-\(.*\)-\(.*\).out/\5/g')"

	grep "PartitionId: " $bifile | grep debug | grep -v FINISH | sed "s/.*PartitionId: //g" | cut -f 1,3,5,9,11,14,16 -d " " | sed "s/\\/125//g"  | sed "s/}//g" | awk -v graph=$graph -v t=$t -v n=$n -v k=$k -v id=$id '{ printf("%s %d %d %d %d %s\n", graph, t, n, k, id,$0) }' >> $CSV_FILE

done 	
	
