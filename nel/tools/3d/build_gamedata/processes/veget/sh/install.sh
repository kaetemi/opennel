#!/bin/bash

# Install shapes in the client data

# Get the shape install directory
shape_install_directory=`cat ../../cfg/directories.cfg | grep "shape_install_directory" | sed -e 's/shape_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the maps install directory
map_install_directory=`cat ../../cfg/directories.cfg | grep "map_install_directory" | sed -e 's/map_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/config.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install Shape >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install Shape 
echo ------- 

cp -u shape/*.shape $client_directory/$shape_install_directory  2>> log.log
cp -u shape_with_coarse_mesh_builded/*.shape $client_directory/$shape_install_directory  2>> log.log
cp -u shape_with_coarse_mesh_builded/*.dds $client_directory/$map_install_directory  2>> log.log
