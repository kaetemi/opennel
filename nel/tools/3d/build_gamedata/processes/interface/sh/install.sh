#!/bin/bash

# Install the interface in the client data

# Get the interface install directory
interface_install_directory=`cat ../../cfg/config.cfg | grep "interface_install_directory" | sed -e 's/interface_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/config.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install interface >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install interface 
echo ------- 

cp -u tga/*.tga $client_directory/$interface_install_directory  2>> log.log
