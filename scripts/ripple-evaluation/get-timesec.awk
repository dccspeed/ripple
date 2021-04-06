{ 
   if (NF == 3) {
	#print $1, $2, $3
	print ($1*3600)+$2*60+$3  
   }
   else if (NF=2) {	
	#print $1, $2
	print $1*60+$2  
   }
}
