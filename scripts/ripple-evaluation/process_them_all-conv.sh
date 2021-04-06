#! /usr/bin/env sh

#rm -rf ./web-Google-convergence/
#./process_dataset_matryoshka2.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-8-without-overlap/ web-Google-sl.gph web-Google-convergence/ 1000
#./process_dataset_motivo2.sh /scratch/cdecarv/tmp/running-time/running-time-tmp-9-motivo/ web-Google-sl.mtv web-Google-convergence/ 1000

#rm -rf ./dblp-convergence/
#./process_dataset_motivo2.sh /scratch1/cdecarv/tmp/convergence_test com-dblp.ungraph-sl.mtv dblp-convergence/ 1000
#./process_dataset_matryoshka2.sh /scratch1/cdecarv/tmp/convergence_test com-dblp.ungraph-sl.gph dblp-convergence/ 1000

rm -rf ./amazon-convergence/
./process_dataset_motivo2.sh /scratch1/cdecarv/tmp/convergence_test com-amazon.ungraph-sl.mtv amazon-convergence/ 1000
./process_dataset_matryoshka2.sh /scratch1/cdecarv/tmp/convergence_test com-amazon.ungraph-sl.gph amazon-convergence/ 1000

#rm -rf ./patents-convergence/
#./process_dataset_motivo2.sh /scratch1/cdecarv/tmp/convergence/ cit-Patents-sl.mtv patents-convergence/ 1000
#./process_dataset_matryoshka2.sh /scratch1/cdecarv/tmp/convergence_72_threads/ cit-Patents-sl.gph patents-convergence/ 1000
