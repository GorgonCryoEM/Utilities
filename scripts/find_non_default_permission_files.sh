# http://serverfault.com/a/49059

find . -path ./.git -prune -o -path ./ExternalLibraries -prune -o  \! -perm 644 -type f -o \! -perm 755 -type d
