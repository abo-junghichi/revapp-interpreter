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
int main(int argc, char **argv)
{
    //const char romsrc[] = ")";
    return main_core(primitives, romsrc, argc, argv);
}
