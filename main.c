#include <stdio.h>
#include "revappi.h"
#include "romsrc.c"
static const thunk_type thunk_int =
    { beta_undefined, thunk_nop_retain, thunk_nop_release };
static const thunk_type *const dist_int[] = { &thunk_int, NULL };
#define BINARYOPINT(OP,name) \
static size_t beta_prim_##name(word *r)\
{\
    r[0].i = r[0].i OP r[1].i;\
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
    int act = r[1].i, pas = r[0].i;
    if (0 == act) {
	r[0].thp = r[3].thp;
	return 1;
    } else {
	r[1].i = pas / act;
	r[0].i = pas % act;
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
    r[0].i = i;
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
    r[0].thp = r[r[0].i OP r[1].i ? 3 : 2].thp;\
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
    fputc(r[1].i, f);
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
    r[1].i = fgetc(stdin);
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
static int place_source(const char *path, const char **sip_p)
{
    cell *cur = ram;
    FILE *file = fopen(path, "r");
    char *sip = ram[0].m;
    size_t brac = 0;
    while (1) {
	char *curp = cur->m;
	size_t i, byte = fread(curp, 1, sizeof(cell), file);
	for (i = 0; i < byte; i++)
	    switch (curp[i]) {
	    case '(':
		brac++;
		break;
	    case ')':
		if (0 >= brac)
		    return 1;
		brac--;
		break;
	    }
	if (sizeof(cell) > byte) {
	    curp[byte] = ')';
	    break;
	}
	cur++;
    }
    fclose(file);
    if (0 < brac)
	return 2;
    cell_allocator_init(cur + 1, &ram[RAM_SIZE], NULL);
#ifdef SHEBANG
    if ('#' == sip[0] && '!' == sip[1]) {
	sip += 2;
	while ('\n' != *(sip++));
    }
#endif				/* SHEBANG */
    *sip_p = sip;
    return 0;
}
int main(int argc, char **argv)
{
    //const char romsrc[] = ")";
    const char *ramsrc;
    int rst;
    if (2 != argc) {
	fprintf(stderr, "%s [source file]\n", argv[0]);
	return 1;
    }
    rst = place_source(argv[1], &ramsrc);
    if (rst)
	return rst + 1;
    return revapp_interp(primitives, romsrc, ramsrc);
}
