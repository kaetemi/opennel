#!/bin/bash

# *** Export vegetset file (.vegetset)

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the vegetset directories
vegetset_source_directories=`cat ../../cfg/directories.cfg | grep "vegetset_source_directory" | sed -e 's/vegetset_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export vegetset >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export vegetset 
echo ------- 

# For each vegetset directory
for i in $vegetset_source_directories ; do
	# Copy
	cp -u $database_directory/$i/*.vegetset vegetset 2>> log.log
done
