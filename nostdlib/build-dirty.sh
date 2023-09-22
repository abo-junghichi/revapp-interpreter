#!/bin/dash
cp ../revappi.h ../revappi.c ../main.c ../romsrc.c .
patch < nostdlib.patch
for f in system-i386 revappi main
do
	gcc -Os -Wall -pedantic -Wextra -Wstrict-aliasing=1 \
	-static -nostdlib -fno-pie $f.c -S -o /dev/stdout | \
	sed 's/.rodata/.text/g' >$f.s
done
gcc -static -nostdlib -fno-pie *.s -o revappi.out
hexcapitalize(){
        cut -f1 -d\  | tr 'abcdef' 'ABCDEF'
}
LINE=$(readelf -l revappi.out | grep 'R E')
OFFSET=$(echo $LINE | cut -f2 -dx | hexcapitalize)
LENGTH=$(echo $LINE | cut -f5 -dx | hexcapitalize)
SIZE=$(echo "ibase=16 ; $OFFSET + $LENGTH" | bc )
dd if=revappi.out of=revappi.out.trunc bs=$SIZE count=1
chmod +x revappi.out.trunc
