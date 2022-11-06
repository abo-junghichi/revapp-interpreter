clean:
	rm letters.out letters.revapp romsrc.revapp
letters.out: letters.c
	gcc -Wall -Wextra -Wstrict-aliasing=1 -pedantic letters.c -o letters.out
letters.revapp: letters.out
	./letters.out >letters.revapp
romsrc.revapp: primitives.revapp list.revapp numbers.revapp letters.revapp iosystem.revapp
	cpp -P -undef <iosystem.revapp >romsrc.revapp
