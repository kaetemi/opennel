#!/bin/bash

# Install maps in the client data

# Get the skel install directory
map_install_directory=`cat ../../cfg/directories.cfg | grep "map_install_directory" | sed -e 's/map_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/config.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install maps >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install maps
echo ------- 

echo install directory = $client_directory /$map_install_directory
cp -u dds/*.dds $client_directory/$map_install_directory  2>> log.log
