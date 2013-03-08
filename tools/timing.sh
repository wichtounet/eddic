#! /bin/bash

#By default, stay in the current directory and used the installed eddic version
executable=${1:-"eddic"}
base_dir=${2:-"."}

#Get the directories
cases_dir="$base_dir/test/cases/*.eddi"

rm -f eddic_times

#Time the test case
for file in $cases_dir ; do
    echo $file
    $executable --64 --O3 --quiet --time $file >> eddic_times
done

samples_dir="$base_dir/eddi_samples/*.eddi"

#Time the samples
for file in $samples_dir ; do
    echo $file
    $executable --64 --O3 --quiet --time $file >> eddic_times
done

#Display results
awk 'BEGIN { FS = " " } ; { print $1 }' eddic_times | sed "s/ms//g" | awk -f tools/aggregate.awk | sort -nr
