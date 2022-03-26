#!/bin/bash

LKC=$(dirname $0)/../bin/lkc

for src in "$@"
do
    if $CBC $src >/dev/null 2>&1
    then
        asm=$(basename $src .cb).s
        echo "$src:"
        $LKC -O -S $src -o $asm.opt &&
        diff -u $asm $asm.opt
    fi
done
