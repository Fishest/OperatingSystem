#!/bin/bash
START_TIME=$(date +%s%N)
for num in {1..100}
do
file=$(printf './test_directory/file%d' $num)
time=$(date +%s%N)
# echo $file
# echo $time > $file
echo $time
rm $file
sleep 0.1
done
ELAPSED_TIME=$((($(date +%s%N) - $START_TIME)/1000000))
echo "Time taken: $ELAPSED_TIME ms"
exit 0
