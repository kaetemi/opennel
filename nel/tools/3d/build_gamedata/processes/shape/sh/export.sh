#!/bin/bash

# *** Export shape files (.shape) from Max

# Get the max directory
max_directory=`cat ../../cfg/config.cfg | grep "max_directory" | sed -e 's/max_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/config.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/config.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the shape directories
shape_source_directories=`cat ../../cfg/config.cfg | grep "shape_source_directory" | sed -e 's/shape_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the options
seoel=`cat ../../cfg/config.cfg | grep "shape_export_opt_export_lighting" | sed -e 's/shape_export_opt_export_lighting//' | sed -e 's/ //g' | sed -e 's/=//g'`
seos=`cat ../../cfg/config.cfg | grep "shape_export_opt_shadow" | sed -e 's/shape_export_opt_shadow//' | sed -e 's/ //g' | sed -e 's/=//g'`
seoll=`cat ../../cfg/config.cfg | grep "shape_export_opt_lighting_limit" | sed -e 's/shape_export_opt_lighting_limit//' | sed -e 's/ //g' | sed -e 's/=//g'`
seols=`cat ../../cfg/config.cfg | grep "shape_export_opt_lumel_size" | sed -e 's/shape_export_opt_lumel_size//' | sed -e 's/ //g' | sed -e 's/=//g'`
seoo=`cat ../../cfg/config.cfg | grep "shape_export_opt_oversampling" | sed -e 's/shape_export_opt_oversampling//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export shape >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export shape 
echo ------- 

# For each directoy

for i in $shape_source_directories ; do
	# Copy the script
	cat maxscript/shape_export.ms | sed -e "s&shape_source_directory&$database_directory/$i&g" | sed -e "s&output_directory_without_coarse_mesh&$build_gamedata_directory/processes/shape/shape&g" | sed -e "s&output_directory_with_coarse_mesh&$build_gamedata_directory/processes/shape/shape_with_coarse_mesh&g" | sed -e "s&shape_export_opt_export_lighting&$seoel&g" | sed -e "s&shape_export_opt_shadow&$seos&g" | sed -e "s&shape_export_opt_lighting_limit&$seoll&g" | sed -e "s&shape_export_opt_lumel_size&$seols&g" | sed -e "s&shape_export_opt_oversampling&$seoo&g" | sed -e "s&shape_lightmap_path&$build_gamedata_directory/processes/shape/lightmap&g" > $max_directory/scripts/shape_export.ms

	# Start max
	$max_directory/3dsmax.exe -U MAXScript shape_export.ms -q -mi

	# Concat log.log files
	cat $max_directory/log.log >> log.log
done
