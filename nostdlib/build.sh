#!/bin/dash
cp ../revappi.h ../revappi.c ../main.c ../romsrc.c .
patch < nostdlib.patch
gcc -Os -Wall -pedantic -Wextra -Wstrict-aliasing=1 -static -nostdlib system-i386.c revappi.c main.c -o revappi.out
