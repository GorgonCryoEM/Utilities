#!/bin/bash

while read line; do
    f=$line
files=( `git ls-tree -r --name-only HEAD | xargs grep -l "CVS Meta Information"` )

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
