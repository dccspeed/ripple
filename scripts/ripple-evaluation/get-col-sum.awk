{
	for(i=1;i<=NF;i++) {
		sum[i] += $i; 
		#values[i][$i]++
	}
}
	
END {
	for (i=1;i<=NF;i++) {
		printf("%lf\t", sum[i])
	}
	printf("\n")

} 


