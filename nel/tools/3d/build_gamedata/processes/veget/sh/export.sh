#!/bin/bash

# *** Export veget files (.veget) from Max

# Get the max directory
max_directory=`cat ../../cfg/config.cfg | grep "max_directory" | sed -e 's/max_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/config.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/config.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the veget directories
veget_source_directories=`cat ../../cfg/directories.cfg | grep "veget_source_directory" | sed -e 's/veget_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export veget >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export veget 
echo ------- 

# For each directoy

for i in $veget_source_directories ; do
	# Copy the script
	cat maxscript/veget_export.ms | sed -e "s&veget_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/veget/veget&g" > $max_directory/scripts/veget_export.ms

	# Start max
	$max_directory/3dsmax.exe -U MAXScript veget_export.ms -q -mi

	# Concat log.log files
	cat $max_directory/log.log >> log.log
done
