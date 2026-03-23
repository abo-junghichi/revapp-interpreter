#include "revappb.c"
#include "system.c"
THUNK_CONST(thunk_undefined, thunk_error_beta);
static const word embed_undefined[] = { { (intptr_t) & thunk_undefined } };
static const thunk_type thunk_int =
    { thunk_error_beta, retain_cell, release_leaf, NULL };
static int getintfromcell(word w, int *dest)
{
    cell *cep = w.p;
    if (&thunk_int != cep->w[0].c)
	return 1;
    *dest = cep->w[2].i;
    return 0;
}
static cell *gencellfromint(int cont)
{
    cell *rtn = cell_alloc();
    rtn->w[0].c = &thunk_int;
    rtn->w[1].i = 0;
    rtn->w[2].i = cont;
    rtn->w[3].i = 0;
    return rtn;
}
#define COMPAREINT(OP, name) \
static int verb_##name(word *regfile, size_t *reg_used)\
{\
    int std, tgt;\
    if (getintfromcell(regfile[0], &std)\
	|| getintfromcell(regfile[1], &tgt))\
	return -1;\
    return !(tgt OP std);\
}\
static const word embed_##name[] = {\
    { (intptr_t) & thunk_verb }, { 2 }, { (intptr_t) & verb_##name }, { 2 }\
}
COMPAREINT( ==, equal);
COMPAREINT(>, big);
COMPAREINT(>=, eqbig);
#define BINARYOP(name) \
static const word embed_##name[] = {\
    { (intptr_t) & thunk_verb }, { 2 }, { (intptr_t) & verb_##name }, { 1 }\
}
#define BINARYOPINT(OP,name) \
static int verb_##name(word *regfile, size_t *reg_used)\
{\
    int act, pas;\
    if (getintfromcell(regfile[0], &act)\
	|| getintfromcell(regfile[1], &pas))\
	return -1;\
    regfile[2].p = gencellfromint(pas OP act);\
    *reg_used = 3;\
    return 0;\
}\
BINARYOP(name)
BINARYOPINT(+, plus);
BINARYOPINT(-, minus);
BINARYOPINT(*, mul);
BINARYOPINT(&, and);
BINARYOPINT(|, or);
BINARYOPINT(^, xor);
BINARYOPINT(>>, shr);
BINARYOPINT(<<, shl);
static int verb_divmod(word *regfile, size_t *reg_used)
{
    int act, pas;
    if (getintfromcell(regfile[0], &act)
	|| getintfromcell(regfile[1], &pas))
	return -1;
    if (0 == act)
	return 0;
    regfile[2].p = gencellfromint(pas / act);
    regfile[3].p = gencellfromint(pas % act);
    *reg_used = 4;
    return 1;
}
static const word embed_divmod[] = {
    { (intptr_t) & thunk_verb }, { 2 }, { (intptr_t) & verb_divmod }, { 2 }
};
THUNK_CONST(thunk_world, thunk_error_beta);
static const word word_world[] = { { (intptr_t) & thunk_world } };
static int checkwordworld(word w)
{
    return &thunk_world != ((word *) w.p)[0].c;
}
static int verb_forkworld(word *regfile, size_t *reg_used)
{
    if (checkwordworld(regfile[0]))
	return -1;
    regfile[1].c = regfile[2].c = word_world;
    *reg_used = 3;
    return 0;
}
static const word embed_forkworld[] = {
    { (intptr_t) & thunk_verb }, { 1 }, { (intptr_t) & verb_forkworld },
    { 1 }
};
static int verb_joinworld(word *regfile, size_t *reg_used)
{
    if (checkwordworld(regfile[0]) || checkwordworld(regfile[1]))
	return -1;
    regfile[2].c = word_world;
    *reg_used = 3;
    return 0;
}
BINARYOP(joinworld);
static const unsigned char bytecode_startworld[] = {
    0x90, 0xb1, 0xb0, 0x02
};
static const word embed_startworld[] = {
    { (intptr_t) & thunk_src_const },
    { (intptr_t) bytecode_startworld + 0 },
    { (intptr_t) word_world }, { 0 }
};
#define PUTC_CORE(file,name) \
static int verb_##name(word *regfile, size_t *reg_used)\
{\
    int c;\
    char buf;\
    if (getintfromcell(regfile[0], &c) || checkwordworld(regfile[1]))\
	return -1;\
    buf = c;\
    my_write(file, &buf, 1);\
    regfile[2].c = word_world;\
    *reg_used = 3;\
    return 0;\
}\
static const word embed_##name[] = {\
    { (intptr_t) & thunk_verb }, { 2 }, { (intptr_t) & verb_##name }, { 1 }\
}
PUTC_CORE(my_stdout, putc);
PUTC_CORE(my_stderr, errc);
static int verb_getc(word *regfile, size_t *reg_used)
{
    unsigned char c;
    int i;
    if (checkwordworld(regfile[0]))
	return -1;
    if (1 != my_read(my_stdin, &c, 1))
	i = EOF;
    else
	i = c;
    regfile[1].p = gencellfromint(i);
    regfile[2].p = word_world;
    *reg_used = 3;
    return 0;
}
static const word embed_getc[] = {
    { (intptr_t) & thunk_verb }, { 1 }, { (intptr_t) & verb_getc },
    { 1 }
};
static const unsigned char code_cover[] = {
    0x90, 0x90, 0xb1, 0xb0, 0x02
};
static const word word_cover[] = {
    { (intptr_t) & thunk_src_const }, { (intptr_t) code_cover + 0 },
    { 0 }
};
static int gen_int_beta(word *stack_cache, word *regfile, force_state *fs)
{
    word blob = ((const word *) fs->w.c)[1];
    cell *cep = cell_alloc();
    cep->w[0].c = &thunk_int;
    cep->w[1].i = 0;
    cep->w[2] = blob;
    cep->w[3].i = 0;
    stack_cache_push(stack_cache, fs, cep->w);
    fs->w.c = word_cover;
    return 0;
}
THUNK_CONST(thunk_gen_int, gen_int_beta);
#define CONST_INT(val, name) \
static const word embed_##name[] =\
    { { (intptr_t) & thunk_gen_int }, { val } }
CONST_INT(0, zero);
CONST_INT(1, one);
CONST_INT( /* EOF */ -1, eof);
#include "constint.c"
