{
	for(i=1;i<=NF;i++) {
		error_relative = (gt-$i)/gt
		#error = sqrt((gt-$i)*(gt-$i))
		#sum[i] += error
		#values[i][error]++
		printf("%lf\t", error_relative)
	}
	printf("\n")

}
	
END {

} 


