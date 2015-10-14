#!/bin/bash

while read line; do
    f=$line
    f_temp=${f}_temp
    echo $f
    awk '
        BEGIN{skip=0}
        /CVS Meta Information/{skip = !skip}
        skip==1 && !(/^\/\// || /#/){skip = !skip}

        skip==0 {print}
    ' $f > $f_temp
    cat $f_temp > $f
    rm $f_temp
done < cvs_meta_info.list
