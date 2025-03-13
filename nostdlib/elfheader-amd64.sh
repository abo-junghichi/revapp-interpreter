#!/bin/dash
subst () {
	sed "s/$1/$2/g"
}
<<END subst TEXT_ADDR $1 | subst TEXT_SIZE $2 \
	| subst BSS_ADDR $3 | subst BSS_SIZE $4
	.file	"elfheader.s"
	.section	.text.startup,"ax",@progbits
	.globl	my_elfheader
	.type	my_elfheader, @function
my_elfheader:
	.byte 0x7f, 'E', 'L', 'F', 2, 1, 1, 0
	.long 0, 0
	.word 2, 0x3e
	.long 1
	.long _start, 0
	.long 64, 0
	.long 0, 0
	.long 0
	.word 64
	.word 0x38
	.word 2
	.word 0
	.word 0
	.word 0
# program header
# load text-page
	.long 1
	.long 5
	.long 0, 0
	.long 0xTEXT_ADDR, 0
	.long 0xTEXT_ADDR, 0
	.long 0xTEXT_SIZE, 0
	.long 0xTEXT_SIZE, 0
	.long 0x1000, 0
# load bss-page
	.long 1
	.long 6
	.long 0, 0
	.long 0xBSS_ADDR, 0
	.long 0xBSS_ADDR, 0
	.long 0, 0
	.long 0xBSS_SIZE, 0
	.long 0x1000, 0
	.size	my_elfheader, .-my_elfheader
	.section	.note.GNU-stack,"",@progbits
END
