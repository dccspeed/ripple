#! /usr/bin/env sh

rm -rf ./facebook-accuracy/
./process_dataset_matryoshka-gt2.sh /scratch1/cdecarv/tmp/accuracy/accuracy-tmp-6/ facebook-mpi-sl.gph ../kdd2020/results/escape/ facebook-links.edges facebook-accuracy/

rm -rf ./dblp-accuracy/
./process_dataset_matryoshka-gt2.sh /scratch1/cdecarv/tmp/accuracy/accuracy-tmp-6/ com-dblp.ungraph-sl.gph ../kdd2020/results/escape/ com-dblp.edges dblp-accuracy/

rm -rf ./PP-Decagon-accuracy/
./process_dataset_matryoshka-gt2.sh /scratch1/cdecarv/tmp/accuracy/accuracy-tmp-extra-results PP-Decagon_ppi.gph ../kdd2020/results/escape/ PP-Decagon_ppi.edges PP-Decagon-accuracy/
