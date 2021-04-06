awk  '{ if ($1 != "#") { for (i=3;i<=NF;i++) { printf("e %d %d 0\n", $1, $i);} } }'  /scratch/carlos/datasets/Qanat/patent_citations_full_qanat.ordered.singlelabel.input > /scratch/carlos/datasets/Qanat/patent_citations_full_qanat.ordered.singlelabel.input.edges
awk  '{ if ($1 != "#") { printf("v %d %d\n", $1, $2); } }'  /scratch/carlos/datasets/Qanat/patent_citations_full_qanat.ordered.singlelabel.input > /scratch/carlos/datasets/Qanat/patent_citations_full_qanat.ordered.singlelabel.input.nodes

