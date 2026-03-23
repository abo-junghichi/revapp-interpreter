#!/bin/sh

CC='gcc -Wall -pedantic -Wextra -Wstrict-aliasing=1'

cp -r ../romsrc .
cp romsrc.patch/* romsrc/
( cd romsrc ; make romsrc.revapp )
mkdir seed
cd seed
for f in revappi.h revappi.c main.c romsrc Makefile img2c.c
do
	cp -r ../../$f .
done
patch -p1 < ../seed.patch
make revappi.out
cp ../seed.revapp comp.revapp
echo 'compile_application main' >>comp.revapp
cat ../romsrc/romsrc.revapp comp.revapp | ./revappi.out comp.revapp \
	>../application.c
cd ..
$CC -O genconstint.c -o genconstint.out
./genconstint.out >constint.c
$CC -O3 main.c -o comp.out
