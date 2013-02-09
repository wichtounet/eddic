#!/bin/bash

TIMEFORMAT='%3R'

ITERATIONS=2

function bench_base(){
    min=0.0

    /tmp/ramdrive/dev/eddic/bin/eddic --quiet --O$1 $2

    for i in `seq 1 $ITERATIONS`
    do
        { time ./a.out ; } 2> tmp

        result=`cat tmp`

        if [[ $result < $min ]]
        then
            min=$result
        fi
        
        if [[ $min  == 0.0 ]]
        then
            min=$result
        fi
    done

    printf "%s O%d min:%0.3f \n" $2 $1 $min

    rm a.out
    rm tmp
}

function bench(){
    file=kernels/$1.eddi

    bench_base 0 $file
    bench_base 1 $file
    bench_base 2 $file
    bench_base 3 $file
}

bench bubble_sort
