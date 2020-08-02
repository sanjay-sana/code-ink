date1=$(date +"%s")
./sender nodeB 4444 /tmp/data.bin 
date2=$(date +"%s")
diff=$(($date2-$date1))
echo "$(($diff / 60)) minutes and $(($diff % 60)) seconds elapsed."

