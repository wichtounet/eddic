BEGIN {
    FS=":";
    OFS=":";
}

/Timers/ {
    next
}

{
    COUNTS[$1] += 1;
    TOTALS[$1] += $2;
}

END {
    for(ID in COUNTS) {
        if(TOTALS[ID] > 0){
            print TOTALS[ID], ID;
        }
    }
}
