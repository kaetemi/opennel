#!/bin/bash

# Install the zonels in the client data

# Get the zone install directory
zone_install_directory=`cat ../../cfg/directories.cfg | grep "zone_install_directory" | sed -e 's/zone_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/config.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install zone >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install zone 
echo ------- 

cp -u zone_lighted/*.* $client_directory/$zone_install_directory  2>> log.log

# copy the water maps once they have been lighted
cp -u water_shapes_lighted/*.* $client_directory/$water_maps_directories



# Install zone ig lighted in the client data

# Get the ig install directory
ig_install_directory=`cat ../../cfg/directories.cfg | grep "ig_install_directory" | sed -e 's/ig_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install Zone Ig >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install Zone Ig 
echo ------- 

cp -u ig_land_lighted/*.ig $client_directory/$ig_install_directory  2>> log.log
