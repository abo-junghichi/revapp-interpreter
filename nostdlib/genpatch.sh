#!/bin/dash
PATCH=nostdlib.patch
rm $PATCH
for f in revappi.h revappi.c main.c
do
	mv $f $f.new
	cp ../$f .
	diff -u $f $f.new >> $PATCH
	mv $f.new $f
done
