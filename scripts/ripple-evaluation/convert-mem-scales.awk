{
	last = substr($0, length($0),1); 
	if (last == "K") 
		print substr($0, 0, length($0)-1);
	else if (last == "M")
		print int(substr($0, 0, length($0)-1))*1024;
	else if (last == "G")
		print int(substr($0, 0, length($0)-1))*1024*1024;
}
