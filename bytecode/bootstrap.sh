#!/bin/sh

CC='gcc -Wall -pedantic -Wextra -Wstrict-aliasing=1'

cp -r ../romsrc .
( cd romsrc ; make clean )
mkdir seed
cp -r romsrc seed/
cp romsrc.patch/* romsrc/
( cd romsrc ; make romsrc.revapp )
(
	cd seed
	for f in revappi.h revappi.c main.c Makefile img2c.c
	do
		cp -r ../../$f .
	done
	patch -p1 < ../seed.patch
	make revappi.out
	cp ../seed.revapp comp.revapp
	echo 'compile_application main' >>comp.revapp
	cat ../romsrc/romsrc.revapp comp.revapp | ./revappi.out comp.revapp \
		>../application.c
)
$CC -O genconstint.c -o genconstint.out
./genconstint.out >constint.c
$CC -O3 main.c -o comp.out
