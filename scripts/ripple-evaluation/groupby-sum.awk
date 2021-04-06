 { 
	 a[$1] += $2 
 }
  END {
    for (i in a) {
      printf "%s %lf\n", i, a[i];
    }
  }
