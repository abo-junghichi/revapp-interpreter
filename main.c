#include <stdio.h>
#include "revappi.h"
#include "romsrc.c"
static const thunk_type thunk_int =
    { beta_undefined, thunk_nop_retain, thunk_nop_release };
static const thunk_type *const dist_int[] = { &thunk_int, NULL };
typedef union {
    word w;
    int i;
} word_converter;
static int word_to_int(word w)
{
    word_converter u;
    u.w = w;
    return u.i;
}
static word int_to_word(int i)
{
    word_converter u;
    u.i = i;
    return u.w;
}
#define BINARYOPINT(OP,name) \
static size_t beta_prim_##name(word *r)\
{\
    r[0] = int_to_word(word_to_int(r[0]) OP word_to_int(r[1]));\
    r[1].thp = r[2].thp;\
    return 0;\
}\
static const prim_src prim_##name[] = {\
    PIP_POP_THUNK, PIP_POP(int), PIP_POP(int),\
    PIP_CALL(name), PIP_DIST(int)\
}
BINARYOPINT(+, plus);
BINARYOPINT(-, minus);
BINARYOPINT(*, mul);
static const thunk_type *const dist_intint[] =
    { &thunk_int, &thunk_int, NULL };
static const thunk_type *const dist_thunk[] = { NULL };
static size_t beta_prim_divmod(word *r)
{
    int act = word_to_int(r[1]), pas = word_to_int(r[0]);
    if (0 == act) {
	r[0].thp = r[3].thp;
	return 1;
    } else {
	r[1] = int_to_word(pas / act);
	r[0] = int_to_word(pas % act);
	return 0;
    }
}
static const prim_src prim_divmod[] = {
    PIP_POP_THUNK, PIP_POP_THUNK, PIP_POP(int), PIP_POP(int),
    PIP_CALL(divmod), PIP_DIST(intint), PIP_DIST(thunk)
};
static size_t beta_primcore_const_int(int i, word *r)
{
    r[1].thp = r[0].thp;
    r[0] = int_to_word(i);
    return 0;
}
#define CONSTINT(value,name) \
static size_t beta_prim_##name(word *r)\
{\
    return beta_primcore_const_int(value, r);\
}\
static const prim_src prim_##name[] =\
    { PIP_POP_THUNK, PIP_CALL(name), PIP_DIST(int) }
CONSTINT(0, zero);
CONSTINT(1, one);
CONSTINT(EOF, eof);
#define COMPAREINT(OP,name) \
static size_t beta_prim_##name(word *r)\
{\
    r[0].thp = r[word_to_int(r[0]) OP word_to_int(r[1]) ? 3 : 2].thp;\
    return 0;\
}\
static const prim_src prim_##name[] = {\
    PIP_POP_THUNK, PIP_POP_THUNK, PIP_POP(int), PIP_POP(int),\
    PIP_CALL(name), PIP_DIST(thunk)\
}
COMPAREINT( ==, equal);
COMPAREINT(>, big);
COMPAREINT(>=, eqbig);
static const thunk_type *const dist_world[] = { &thunk_world, NULL };
static size_t beta_prim_startworld(word *r)
{
    r[1].thp = r[0].thp;
    return 0;
}
static const prim_src prim_startworld[] =
    { PIP_POP_THUNK, PIP_CALL(startworld), PIP_DIST(world) };
static const thunk_type *const dist_forkworld[] =
    { &thunk_world, &thunk_world, NULL };
static size_t beta_prim_forkworld(word *r)
{
    r[2].thp = r[1].thp;
    return 0;
}
static const prim_src prim_forkworld[] = {
    PIP_POP_THUNK, PIP_POP(world), PIP_CALL(forkworld), PIP_DIST(forkworld)
};
static size_t beta_prim_joinworld(word *r)
{
    r[1].thp = r[2].thp;
    return 0;
}
static const prim_src prim_joinworld[] = {
    PIP_POP_THUNK, PIP_POP(world), PIP_POP(world), PIP_CALL(joinworld),
    PIP_DIST(world)
};
static size_t beta_primcore_putc(FILE *f, word *r)
{
    fputc(word_to_int(r[1]), f);
    r[1].thp = r[2].thp;
    return 0;
}
#define PUTC_CORE(file,name) \
static size_t beta_prim_##name(word *r)\
{\
    return beta_primcore_putc(file, r);\
}\
static const prim_src prim_##name[] = {\
    PIP_POP_THUNK, PIP_POP(int), PIP_POP(world),\
    PIP_CALL(name), PIP_DIST(world)\
}
PUTC_CORE(stdout, putc);
PUTC_CORE(stderr, errc);
static size_t beta_prim_getc(word *r)
{
    r[2].thp = r[1].thp;
    r[1] = int_to_word(fgetc(stdin));
    return 0;
}
static const thunk_type *const dist_getc[] =
    { &thunk_world, &thunk_int, NULL };
static const prim_src prim_getc[] =
    { PIP_POP_THUNK, PIP_POP(world), PIP_CALL(getc), PIP_DIST(getc) };
static const prim_env_member primitives[] = {
    PRIMITIVE(plus), PRIMITIVE(minus), PRIMITIVE(mul), PRIMITIVE(divmod),
    PRIMITIVE(zero), PRIMITIVE(one), PRIMITIVE(eof),
    PRIMITIVE(equal), PRIMITIVE(big), PRIMITIVE(eqbig),
    PRIMITIVE(startworld), PRIMITIVE(forkworld), PRIMITIVE(joinworld),
    PRIMITIVE(putc),
#ifndef ALLOC_TRACE
    PRIMITIVE(errc),
#endif
    PRIMITIVE(getc),
    PRIMITIVE_END
};
#define RAM_SIZE 100000
static cell ram[RAM_SIZE];
static int place_source(const char *path)
{
    FILE *file = fopen(path, "r");
    char *sip = ram[0].m;
    size_t brac = 0;
    int linehead = 1;
    while (1) {
	if (1 != fread(sip, 1, 1, file))
	    goto end_of_file;
	switch (*sip) {
	case '(':
	    brac++;
	    break;
	case ')':
	    if (0 >= brac)
		return 1;
	    brac--;
	    break;
	case '#':
	    if (0 == linehead)
		goto skip_clear_linehead;
	    while ('\n' != *sip)
		if (1 != fread(sip, 1, 1, file))
		    goto end_of_file;
	case '\n':
	    linehead = 1;
	    goto skip_clear_linehead;
	}
	linehead = 0;
      skip_clear_linehead:
	sip++;
    }
  end_of_file:
    *sip++ = ')';
    fclose(file);
    if (0 < brac)
	return 2;
    cell_allocator_init((cell *) ((((size_t) sip) + sizeof(cell) - 1) &
				  -sizeof(cell)), &ram[RAM_SIZE], NULL);
    return 0;
}
int main(int argc, char **argv)
{
    //const char romsrc[] = ")";
    const char *ramsrc = ram[0].m;
    int rst;
    if (2 != argc) {
	fprintf(stderr, "%s [source file]\n", argv[0]);
	return 1;
    }
    rst = place_source(argv[1]);
    if (rst)
	return rst + 1;
    return revapp_interp(primitives, romsrc, ramsrc);
}
