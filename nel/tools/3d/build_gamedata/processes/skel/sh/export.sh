#!/bin/bash

# *** Export skeleton files (.skel) from Max

# Get the max directory
max_directory=`cat ../../cfg/site.cfg | grep "max_directory" | sed -e 's/max_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the skel directories
skel_source_directories=`cat ../../cfg/directories.cfg | grep "skel_source_directory" | sed -e 's/skel_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export skeleton >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export skeleton 
echo ------- 

# For each directoy

for i in $skel_source_directories ; do
	# Copy the script
	cat maxscript/skel_export.ms | sed -e "s&skel_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/skel/skel&g" > $max_directory/scripts/skel_export.ms

	# Start max
	$max_directory/3dsmax.exe -U MAXScript skel_export.ms -q -mi -vn

	# Concat log.log files
	cat $max_directory/log.log >> log.log
done
