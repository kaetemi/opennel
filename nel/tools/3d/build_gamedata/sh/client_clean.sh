#!/bin/bash

# Clean the client

# Get the client directory
client_directory=`cat cfg/config.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client setup directories
client_setup_directories=`cat cfg/config.cfg | grep "client_setup_directory" | sed -e 's/client_setup_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* > log.log
echo \*\*\*\*\*\*\* CLIENT CLEAN>> log.log
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* >> log.log
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*
echo \*\*\*\*\*\*\* CLIENT CLEAN
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*

# For each directory
for i in $client_setup_directories ; do
	# Create the directory
	rm $client_directory/$i/*
done