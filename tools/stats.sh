#! /bin/bash

#By default, stay in the current directory and used the installed eddic version
executable=${1:-"eddic"}
base_dir=${2:-"."}

count_ltac=0
count_mtac=0

#Get the directories
sample_dir="$base_dir/samples/*.eddi"
cases_dir="$base_dir/test/cases/*.eddi"

#This sample should be avoided for performance reasons
big_sample="$base_dir/samples/big.eddi"

#Count LTAC Statements
for file in $sample_dir ; do
    if [ "$file" != "$big_sample" ] ; then
        count="`$executable --quiet --ltac-only $file | wc -l`"
        let "count_ltac += $count"
    fi
done

echo "Total LTAC Statements: $count_ltac"

#Count MTAC Statements
for file in $cases_dir ; do
    count="`$executable --quiet --mtac-only $file | wc -l`"
    let "count_mtac += $count"
done

echo "Total MTAC Statements: $count_mtac"
