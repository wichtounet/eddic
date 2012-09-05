#! /bin/bash

#By default, stay in the current directory and used the installed eddic version
executable=${1:-"eddic"}
base_dir=${2:-"."}

#Get the directories
cases_dir="$base_dir/test/cases/*.eddi"

rm cases

#Count Statements in test cases
for file in $cases_dir ; do
    $executable --mtac-only $file >> cases
    
    echo "" >> cases;
done
