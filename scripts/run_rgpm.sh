#! /usr/bin/env sh

BIN_DIR=$1
CONFIG_DIR=$2
OUTPUT_DIR=$3
DATA_DIR=~/rgpm/data/

mkdir -pv $3

for config_file in `ls $CONFIG_DIR`; 
do
	for i in `seq 100`;
	do
		echo "$BIN_DIR/app -i $DATA_DIR/yeastbigcomp-sl.gph -o bla -c $CONFIG_DIR/$config_file -q $DATA_DIR/query-triangle.gph > $OUTPUT_DIR/output_${i}_$config_file" 
		$BIN_DIR/app -i $DATA_DIR/yeastbigcomp-sl.gph -o bla -c $CONFIG_DIR/$config_file -q $DATA_DIR/query-triangle.gph > $OUTPUT_DIR/output_${i}_$config_file 
	done
done
