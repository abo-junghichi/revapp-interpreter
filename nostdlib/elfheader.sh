#!/bin/dash
subst () {
	sed "s/$1/$2/g"
}
<<END subst TEXT_ADDR $1 | subst TEXT_SIZE $2 \
	| subst BSS_ADDR $3 | subst BSS_SIZE $4
	.file   "elfheader.s"
        .text
	.globl  my_elfheader
	.type   my_elfheader, @function
my_elfheader:
	.byte 0x7f, 'E', 'L', 'F', 1, 1, 1, 0
	.long 0, 0
	.word 2, 3
	.long 1
	.long _start
	.long 52
	.long 0
	.long 0
	.word 52
	.word 0x20
	.word 2
	.word 0
	.word 0
	.word 0
# program header
# load text-page
	.long 1
	.long 0
	.long 0xTEXT_ADDR
	.long 0xTEXT_ADDR
	.long 0xTEXT_SIZE
	.long 0xTEXT_SIZE
	.long 5
	.long 0x1000
# load bss-page
	.long 1
	.long 0
	.long 0xBSS_ADDR
	.long 0xBSS_ADDR
	.long 0
	.long 0xBSS_SIZE
	.long 6
	.long 0x1000
	.size  my_elfheader, .-my_elfheader
        .section        .note.GNU-stack,"",@progbits
END
