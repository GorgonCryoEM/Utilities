#!/bin/sh

exclude_dirs=( .git ExternalLibraries )

for d in ${exclude_dirs[@]};do
	prune_str="${prune_str[@]} -not ( -path ./$d -prune )"
done

find . ${prune_str[@]} -type f \! -perm 644 | xargs chmod 644
find . ${prune_str[@]} -type d \! -perm 755 | xargs chmod 755
