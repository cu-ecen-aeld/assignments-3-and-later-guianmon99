#!/bin/bash

if [ $# -ne 2 ]
	then 
		echo "not enoght arguments"
		exit 1 
fi

writefile=$1
writestr=$2

if [ ! -n $writestr ]
	then 
		echo "incorrect arguments"
		exit 1 
fi

mkdir -p $(dirname "$writefile" ) && touch "$writefile" 

echo $writestr > $writefile




