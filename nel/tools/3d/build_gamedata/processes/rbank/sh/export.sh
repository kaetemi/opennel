#!/bin/bash

# *** Export cmb files (.cmb) from Max

# Get the max directory
max_directory=`cat ../../cfg/site.cfg | grep "max_directory" | sed -e 's/max_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the collision directories
collision_source_directories=`cat ../../cfg/directories.cfg | grep "collision_source_directory" | sed -e 's/collision_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export cmb for rbank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export cmb for rbank
echo ------- 

# For each directoy

for i in $collision_source_directories ; do
	# Copy the script
	cat maxscript/rbank_export.ms | sed -e "s&collision_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/rbank/cmb&g" > $max_directory/scripts/rbank_export.ms

	# Start max
	$max_directory/3dsmax.exe -U MAXScript rbank_export.ms -q -mi -vn

	# Concat log.log files
	cat $max_directory/log.log >> log.log
done
