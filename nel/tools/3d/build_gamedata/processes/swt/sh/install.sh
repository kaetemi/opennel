#!/bin/bash

# Install swt in the client data

# Get the swt install directory
swt_install_directory=`cat ../../cfg/config.cfg | grep "swt_install_directory" | sed -e 's/swt_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/config.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install skeleton weight >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install skeleton weight 
echo ------- 

cp -u swt/*.swt $client_directory/$swt_install_directory  2>> log.log
