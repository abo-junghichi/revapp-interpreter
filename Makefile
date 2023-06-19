CC=gcc -Wall -Wextra -Wstrict-aliasing=1 -pedantic
BUILDCC=$(CC) -O
HOSTCC=$(CC) -O3 -DNDEBUG
#HOSTCC=$(CC) -g -DALLOC_TRACE

clean:
	rm *.out *.o romsrc.c
	(cd romsrc ; make)
all: revappi.out

romsrc/romsrc.revapp:
	(cd romsrc ; make romsrc.revapp)
img2c.out: img2c.c
	$(BUILDCC) img2c.c -o img2c.out
romsrc.c: img2c.out romsrc/romsrc.revapp
	./img2c.out < romsrc/romsrc.revapp > romsrc.c
revappi.o: revappi.h revappi.c
	$(HOSTCC) -c revappi.c -DSHEBANG
main.o: revappi.h romsrc.c main.c
	$(HOSTCC) -c main.c
revappi.out: revappi.o main.o
	$(HOSTCC) revappi.o main.o -o revappi.out
