#!/bin/sh
count=0
sum=0
for param in "$@"
do
sum=$(($sum + $param))
count=$(( $count + 1 ))
done
echo "Count=$count"
echo "Sum=$sum"