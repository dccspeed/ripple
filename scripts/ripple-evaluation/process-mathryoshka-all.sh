#! /usr/bin/env bash
required="log_dir dataset escape_dir escape_dataset output_dir"

for argname in $required; do
	if [ -z ${!argname+x} ]; then
		printf "error: $argname is unset\n"
		printf "$wholeusage\n"
		exit 1
	else
		echo "info: $argname is set to '${!argname}'"
	fi
done

rm -rf $output_dir/*
./process_dataset_matryoshka-gt-test.sh $log_dir $dataset $escape_dir $escape_dataset $output_dir
./process_dataset_matryoshka-gt2-test.sh $log_dir $dataset $escape_dir $escape_dataset $output_dir
