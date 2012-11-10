#! /bin/bash

#By default, stay in the current directory and used the installed eddic version
executable=${1:-"eddic"}
base_dir=${2:-"."}

#Get the directories
cases_dir="$base_dir/test/cases/*.eddi"

rm -f tmp_ltac_cases
rm -f tmp_mtac_cases

#Count Statements in test cases
for file in $cases_dir ; do
    $executable --ltac-only $file >> tmp_ltac_cases
    $executable --mtac-only $file >> tmp_mtac_cases
    
    echo "" >> tmp_ltac_cases;
    echo "" >> tmp_mtac_cases;
done
