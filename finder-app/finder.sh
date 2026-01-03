#!/bin/sh

expected_param_nbr=2

if [ $expected_param_nbr -ne $# ]
	then 
		echo "incorrect number of parameters"
		exit 1
fi

filesdir=$1
searchstr=$2

if [ ! -d $filesdir ] && [ ! -d $searchstr ] 
	then
		echo "filesdir must be a directory"
		exit 1
fi

echo $filesdir

number_of_files=$(find $filesdir -type f  | wc -l)
matching_lines_nbr=$(grep -r $searchstr $filesdir | wc -l)

echo "The number of files are ${number_of_files} and the number of matching lines are ${matching_lines_nbr}"

