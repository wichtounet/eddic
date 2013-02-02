BEGIN {
    FS=":";
    OFS=":";
}

/Timers/ { next }
/Total/ { next }
/whole_optimizations/ { next }
/all_optimizations/ { next }

{
    COUNTS[$1] += 1;
    TOTALS[$1] += $2;
}

END {
    GLOBAL_TOTAL = 0;
    for(ID in COUNTS) {
        GLOBAL_TOTAL += TOTALS[ID];
    }

    for(ID in COUNTS) {
        if(TOTALS[ID] > 0){
            print TOTALS[ID], ID, (100 * (TOTALS[ID] / GLOBAL_TOTAL));
        }
    }

    print "Total", GLOBAL_TOTAL;
}
