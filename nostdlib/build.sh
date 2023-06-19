#!/bin/dash
for f in revappi.h revappi.c main.c romsrc.c
do
	cp ../$f .
done
patch < nostdlib.patch
gcc -Os -Wall -pedantic -Wextra -Wstrict-aliasing=1 -static -nostdlib system-i386.c revappi.c main.c -o revappi.out
