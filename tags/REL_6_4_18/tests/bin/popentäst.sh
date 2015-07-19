#!/bin/sh
rm -rf popentest2.out
touch popentest2.tmp
while read line
do
    echo $line >> popentest2.tmp
done
