#! /usr/bin/env sh

for i in `seq 2`;
do 
	cp sub-k3.$i sub-k3.$i.escape.gph
	cp sub-k3.$i.eps sub-k3.$i.escape.eps
	echo "sub-k3.$i.escape.gph"
done

for i in `seq 6`;
do 
	cp sub-k4.$i sub-k4.$i.escape.gph
	cp sub-k4.$i.eps sub-k4.$i.escape.eps
	echo "sub-k4.$i.escape.gph"
done

for i in `seq 21`;
do
	#echo "sub-k5.$i.gph"
	e=`grep "^$i " map_escape.txt | cut -f 3 -d " "`
	cp sub-k5.$i sub-k5.$e.escape.gph
	cp sub-k5.$i.eps sub-k5.$e.escape.eps
	echo "sub-k5.$e.escape.gph"
done
