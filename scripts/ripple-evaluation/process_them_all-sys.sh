#! /usr/bin/env sh

rm -rf ./facebook-sys
./process_dataset_matryoshka-sys.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-8-without-overlap/ facebook-mpi-sl.gph facebook-sys/
./process_dataset_motivo-sys.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-9-motivo/ facebook-mpi-sl.mtv facebook-sys/

rm -rf ./dblp-sys
./process_dataset_matryoshka-sys.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-8-without-overlap/ com-dblp.ungraph-sl.gph dblp-sys/
./process_dataset_motivo-sys.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-9-motivo/ com-dblp.ungraph-sl.mtv dblp-sys/

rm -rf ./amazon-sys
./process_dataset_matryoshka-sys.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-8-without-overlap/ com-amazon.ungraph-sl.gph amazon-sys/
./process_dataset_motivo-sys.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-9-motivo/ com-amazon.ungraph-sl.mtv amazon-sys/

rm -rf ./web-Google-sys
./process_dataset_matryoshka-sys.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-8-without-overlap/ web-Google-sl.gph web-Google-sys/
./process_dataset_motivo-sys.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-9-motivo/ web-Google-sl.mtv web-Google-sys/

rm -rf ./cit-Patents-sys
./process_dataset_matryoshka-sys.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-8-without-overlap/ cit-Patents-sl.gph cit-Patents-sys/
./process_dataset_motivo-sys.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-9-motivo/ cit-Patents-sl.mtv cit-Patents-sys/
