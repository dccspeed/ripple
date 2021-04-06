{
	for(i=1;i<=NF;i++) {
		sum[i] += $i; 
		values[i][$i]++
	}
}
	
END {
	for (i=1;i<=NF;i++) {
		mean[i]=sum[i]/NR
		#printf("mean of column %d: %lf\n", i, mean[i])
		printf("%lf\t", mean[i])
		for (v in values[i]) {
			sum_var[i] += (mean[i]-v)*(mean[i]-v)
		}
		var[i]=sum_var[i]/(NR-1)
		#printf("stdesv of column %d: %lf\n", i, sqrt(var[i]))
  	}
	printf("\n")
	for (i=1;i<=NF;i++) {
		printf("%lf\t", sqrt(var[i]))
	}
	printf("\n")
	#for (i=1;i<=NF;i++) {
	#	printf("%lf\t", sum[i])
	#}
	#printf("\n")

} 


