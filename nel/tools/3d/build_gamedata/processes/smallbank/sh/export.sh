#!/bin/bash

# *** Export bank file (.bank) from Max

# Get the database directory
database_directory=`cat ../../cfg/config.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the swt directories
bank_source_directory=`cat ../../cfg/directories.cfg | grep "bank_source_directory" | sed -e 's/bank_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export bank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export bank 
echo ------- 

# Copy the bank
cp -u $database_directory/$bank_source_directory/*.bank bank 2>> log.log
