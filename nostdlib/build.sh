#!/bin/sh
ARCH=$1
cp ../revappi.h ../revappi.c ../main.c ../romsrc.c .
patch < nostdlib.patch
for f in system-$ARCH revappi main
do
	gcc -Os -Wall -pedantic -Wextra -Wstrict-aliasing=1 \
		-static -nostdlib -fno-builtin -fno-pie -S \
		$f.c -o $f-raw.s
	sed 's/.rodata/.text/g' <$f-raw.s >$f.s
done
sh elfheader-$ARCH.sh 0 0 0 0 > elfheader.s
assemble(){
	gcc -static -nostdlib -fno-builtin -fno-pie \
		elfheader.s system-$ARCH.s revappi.s main.s -o revappi.out
}
assemble
TEXT=$(readelf -l -W revappi.out | grep 'LOAD .* R E')
BSS=$(readelf -l -W revappi.out | grep 'LOAD .* RW ')
ph_cut(){
        cut -f$1 -dx | cut -f1 -d\ 
}
TEXT_ADDR=$(echo $TEXT | ph_cut 3)
TEXT_SIZE=$(echo $TEXT | ph_cut 6)
BSS_ADDR=$(echo $BSS | ph_cut 3)
BSS_SIZE=$(echo $BSS | ph_cut 6)
sh elfheader-$ARCH.sh $TEXT_ADDR $TEXT_SIZE $BSS_ADDR $BSS_SIZE > elfheader.s
assemble
# LENGTH=$(echo $TEXT_SIZE | tr 'abcdef' 'ABCDEF')
# SIZE=$(echo "ibase=16 ; $LENGTH" | bc)
SIZE=$(printf %d 0x$TEXT_SIZE)
dd if=revappi.out of=revappi.out.trunc bs=4096 skip=1
truncate --size=$SIZE revappi.out.trunc
chmod +x revappi.out.trunc
