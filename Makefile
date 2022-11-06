CC=gcc -Wall -Wextra -Wstrict-aliasing=1 -pedantic
BUILDCC=$(CC) -O3
HOSTCC=$(CC) -O0 -g

clean:
	rm *.out romsrc.c
	(cd romsrc ; make)
all: revappi.out

img2c.out: img2c.c
	$(BUILDCC) img2c.c -o img2c.out
romsrc/romsrc.revapp:
	(cd romsrc ; make romsrc.revapp)
romsrc.c: img2c.out romsrc/romsrc.revapp
	./img2c.out < romsrc/romsrc.revapp > romsrc.c
revappi.out: revappi.c romsrc.c
	$(HOSTCC) revappi.c -o revappi.out -DSHEBANG #-DALLOC_TRACE
