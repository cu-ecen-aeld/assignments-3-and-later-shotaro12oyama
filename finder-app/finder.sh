#!/bin/sh
#Assignment1
##Accepts the following runtime arguments: 
##the first argument is a path to a directory on the filesystem, referred to below as filesdir; 
##the second argument is a text string which will be searched within these files, referred to below as searchstr
##Exits with return value 1 error and print statements if any of the parameters above were not specified
##Exits with return value 1 error and print statements if filesdir does not represent a directory on the filesystem
##Prints a message 
##"The number of files are X and the number of matching lines are Y" where X is the number of files in the directory 
##and all subdirectories and Y is the number of matching lines found in respective files, where a matching line refers
##to a line which contains searchstr (and may also contain additional content).

filesdir=$1
searchstr=$2

if [ $# -lt 2 ]
then
	echo "Parameters were not specified"
	exit 1
else
	if [ -d "${filesdir}" ]
	then
		X=`find ${filesdir} | wc -l` && X=$(($X-1))
	   	Y=`grep ${searchstr} -rl ${filesdir} | wc -l`     
		echo "The number of files are ${X} and the number of matching lines are ${Y}"
	else
		echo "Argument 1 does not represent a directory"
		exit 1
	fi
fi


