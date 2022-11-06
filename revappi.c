#include <assert.h>
#include <stdio.h>
typedef union {
    void *p;
    size_t s;
    char m[1];
    int i;
} word;
typedef struct {
    word w[4];
} cell;
#define RAM_SIZE (100000)
cell ram[RAM_SIZE];
cell *free_cell = NULL, *cell_unused;
static void cell_free(cell * cp)
{
    cp->w[0].p = free_cell;
    free_cell = cp;
#ifdef ALLOC_TRACE
    fprintf(stderr, "@ a.out - %p\n", (void *) cp);
#endif
}
static cell *cell_alloc_core(void)
{
    cell *rtn = free_cell;
    if (NULL == rtn)
	rtn = cell_unused++;
    else
	free_cell = rtn->w[0].p;
#ifdef ALLOC_TRACE
    fprintf(stderr, "@ a.out + %p 0x%x\n", (void *) rtn, sizeof(cell));
#endif
    return rtn;
}
static cell *cell_alloc(void)
{
    cell *rtn = cell_alloc_core();
    rtn->w[0].s = 0;
    return rtn;
}
static void cell_retain(cell * cp)
{
    cp->w[0].s++;
}
static void trampoline(void *ip)
{
    while (NULL != ip)
	ip = ((void *(*)(void)) ip)();
}
static void *trampoline_exit(void)
{
    return NULL;
}
typedef struct thunk_type thunk_type;
struct thunk_type {
    void *(*force)(const thunk_type * tht, word w2, word w3);
    void *(*beta)(cell * stack, word w2, word w3);
    void (*copy)(word w2, word w3);
    void *(*release)(cell * thunk);
};
static struct {
    void *link;
    void *dump;
    cell *obj;
} gc_regfile;
static void gc_call(void *ip, cell * obj)
{
    gc_regfile.link = trampoline_exit;
    gc_regfile.obj = obj;
    return trampoline(ip);
}
static void *thunk_release_entry(void);
static void *args_release_entry(void);
static void *env_release_entry(void)
{
    return args_release_entry();
}
static void env_release(cell * env)
{
    gc_call(env_release_entry, env);
}
static void *args_release_1(void)
{
    cell *args = gc_regfile.dump;
    gc_regfile.obj = args->w[3].p;
    gc_regfile.link = args->w[0].p;
    gc_regfile.dump = args->w[1].p;
    cell_free(args);
    return thunk_release_entry;
}
static void *args_release_entry(void)
{
    cell *args = gc_regfile.obj;
    if (NULL == args)
	return gc_regfile.link;
    if (0 < args->w[0].s) {
	args->w[0].s--;
	return gc_regfile.link;
    }
    gc_regfile.obj = args->w[1].p;
    args->w[0].p = gc_regfile.link;
    args->w[1].p = gc_regfile.dump;
    gc_regfile.dump = args;
    gc_regfile.link = args_release_1;
    return args_release_entry;
}
static void args_release(cell * args)
{
    gc_call(args_release_entry, args);
}
static void *thunk_release_entry(void)
{
    cell *thunk = gc_regfile.obj;
    if (0 < thunk->w[0].s) {
	thunk->w[0].s--;
	return gc_regfile.link;
    }
    return ((thunk_type *) thunk->w[1].p)->release(thunk);
}
static void *thunk_src_release_core(void *w2_release, cell * thunk)
{
    gc_regfile.obj = thunk->w[2].p;
    cell_free(thunk);
    return w2_release;
}
static void *thunk_prim_release(cell * thunk)
{
    return thunk_src_release_core(args_release_entry, thunk);
}
static void *thunk_src_release(cell * thunk)
{
    return thunk_src_release_core(env_release_entry, thunk);
}
static void *thunk_error_release_1(void)
{
    cell *thunk = gc_regfile.dump;
    gc_regfile.link = thunk->w[0].p;
    gc_regfile.dump = thunk->w[2].p;
    gc_regfile.obj = thunk->w[3].p;
    cell_free(thunk);
    return thunk_release_entry;
}
static void *thunk_error_release(cell * thunk)
{
    gc_regfile.obj = thunk->w[2].p;
    thunk->w[0].p = gc_regfile.link;
    thunk->w[2].p = gc_regfile.dump;
    gc_regfile.dump = thunk;
    gc_regfile.link = thunk_error_release_1;
    return args_release_entry;
}
static void *thunk_nop_release(cell * thunk)
{
    cell_free(thunk);
    return gc_regfile.link;
}
static const thunk_type thunk_src, thunk_prim, thunk_error;
static struct {
    void *link;
    cell *dump, *thunk;
    const thunk_type *tht;
    word w2, w3;
} force_regfile;
static void force_end_help(const thunk_type * tht, word w2, word w3)
{
    force_regfile.tht = tht;
    force_regfile.w2 = w2;
    force_regfile.w3 = w3;
}
static void *force_end(const thunk_type * tht, word w2, word w3)
{
    force_end_help(tht, w2, w3);
    return force_regfile.link;
}
static void *force_entry(void)
{
    cell *thunk = force_regfile.thunk;
    const thunk_type *tht = thunk->w[1].p;
    word w2 = thunk->w[2], w3 = thunk->w[3];
    if (0 < thunk->w[0].s) {
	thunk->w[0].s--;
	return tht->force(tht, w2, w3);
    } else {
	cell_free(thunk);
	return force_end(tht, w2, w3);
    }
}
static void force(cell * thunk)
{
    force_regfile.link = trampoline_exit;
    force_regfile.thunk = thunk;
    trampoline(force_entry);
}
static void *beta_end_core(const thunk_type * tht, word w2, word w3)
{
    cell *thunk = force_regfile.thunk;
    thunk->w[1].p = tht;
    thunk->w[2] = w2;
    thunk->w[3] = w3;
    tht->copy(w2, w3);
    return force_regfile.link;
}
static void *beta_end(const thunk_type * tht, word w2, word w3)
{
    force_end_help(tht, w2, w3);
    return beta_end_core(tht, w2, w3);
}
static void *beta_body(void)
{
    cell *thunk = force_regfile.dump, *stack = thunk->w[3].p;
    force_regfile.thunk = thunk;
    force_regfile.link = thunk->w[1].p;
    force_regfile.dump = thunk->w[2].p;
    return force_regfile.tht->beta(stack, force_regfile.w2,
				   force_regfile.w3);
}
static void *beta(cell * stack, cell * continuation)
{
    cell *thunk = force_regfile.thunk;
    thunk->w[1].p = force_regfile.link;
    thunk->w[2].p = force_regfile.dump;
    thunk->w[3].p = stack;
    force_regfile.link = beta_body;
    force_regfile.dump = thunk;
    force_regfile.thunk = continuation;
    return force_entry;
}
static void *beta_error_core(cell * stack, const thunk_type * tht, word w2,
			     word w3)
{
    cell *tmp = cell_alloc();
    tmp->w[1].p = tht;
    tmp->w[2] = w2;
    tmp->w[3] = w3;
    w2.p = stack;
    w3.p = tmp;
    return beta_end(&thunk_error, w2, w3);
}
static void *beta_prim(cell * stack, word w2, word w3)
{
    word *pip = w3.p;
    return ((void *(*)(cell *, cell *, word *)) pip->p)(stack, w2.p, pip);
}
static void *beta_prim_core(int blob_mode, cell * stack, cell * args,
			    word * pip)
{
    word w2, w3;
    thunk_type *tgt;
    cell *tos = stack;
    w2.p = args, w3.p = pip;
    if (NULL == stack)
	return beta_end(&thunk_prim, w2, w3);
    if (blob_mode) {
	tgt = ((cell *) stack->w[3].p)->w[1].p;
	pip++;
	if (tgt != pip->p)
	    return beta_error_core(stack, &thunk_prim, w2, w3);
    }
    stack = tos->w[1].p;
    tos->w[1].p = args;
    pip++;
    return ((void *(*)(cell *, cell *, word *)) pip->p)(stack, tos, pip);
}
static void *beta_prim_blob(cell * stack, cell * args, word * pip)
{
    return beta_prim_core(1, stack, args, pip);
}
static void *beta_prim_thunk(cell * stack, cell * args, word * pip)
{
    return beta_prim_core(0, stack, args, pip);
}
static int is_delimiter(const char c)
{
    switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case '=':
    case '(':
    case ')':
	return 1;
    default:
	return 0;
    }
}
static void skip_stem(const char **sip_p)
{
    const char *sip = *sip_p;
    while (!is_delimiter(*++sip));
    *sip_p = sip;
}
static int search_env(const char *sip, cell * env, cell ** found)
{
    for (; NULL != env; env = env->w[1].p) {
	const char *std = sip, *tgt = env->w[2].p;
	while (!is_delimiter(*std)) {
	    if (*std != *tgt)
		goto next;
	    std++;
	    tgt++;
	}
	if (is_delimiter(*tgt)) {
	    *found = env->w[3].p;
	    return 1;
	}
      next:;
    }
    return 0;
}
static cell *exec_src_closure_env(cell * env_parent, const char **sip_p)
{
    const char *sip = *sip_p;
    size_t depth = 0;
    cell *env = NULL, *mask = NULL;
    while (1) {
	cell *tmp;
	switch (*sip) {
	case ' ':
	case '\t':
	case '\n':
	    sip++;
	    break;
	case '=':
	    tmp = cell_alloc_core();
	    tmp->w[0].s = depth;
	    tmp->w[1].p = mask;
	    tmp->w[2].p = sip + 1;
	    mask = tmp;
	    skip_stem(&sip);
	    break;
	case ')':
	    while (NULL != mask && depth <= mask->w[0].s) {
		tmp = mask->w[1].p;
		cell_free(mask);
		mask = tmp;
	    }
	    sip++;
	    if (0 >= depth)
		goto done;
	    depth--;
	    break;
	case '(':
	    sip++;
	    depth++;
	    break;
	default:
	    if (search_env(sip, mask, &tmp))
		goto ref_done;
	    if (search_env(sip, env, &tmp))
		goto ref_done;
	    if (search_env(sip, env_parent, &tmp)) {
		cell *toe = cell_alloc();
		toe->w[1].p = env;
		toe->w[2].p = sip;
		toe->w[3].p = tmp;
		cell_retain(tmp);
		env = toe;
	    }
	  ref_done:
	    skip_stem(&sip);
	    break;
	}
    }
  done:
    *sip_p = sip;
    return env;
}
static void env_retain(cell * env)
{
    if (NULL != env)
	cell_retain(env);
}
static void args_retain(cell * args)
{
    env_retain(args);
}
enum { exec_src_nop = 0, exec_src_func };
static int exec_src(cell ** stack_p, cell ** env_p, const char **sip_p)
{
    int rtn;
    cell *stack = *stack_p, *env = *env_p;
    const char *sip = *sip_p;
    while (1) {
	cell *tmp, *arg;
	switch (*sip) {
	case ' ':
	case '\t':
	case '\n':
	    sip++;
	    break;
	case '=':
	    if (stack) {
		tmp = stack;
		stack = tmp->w[1].p;
		tmp->w[1].p = env;
		tmp->w[2].p = sip + 1;
		env = tmp;
		skip_stem(&sip);
		break;
	    } else {
		rtn = exec_src_func;
		goto end;
	    }
	case ')':
	    rtn = exec_src_nop;
	    goto end;
	case '(':
	    arg = cell_alloc();
	    arg->w[1].p = &thunk_src;
	    arg->w[3].p = ++sip;
	    arg->w[2].p = exec_src_closure_env(env, &sip);
	  push:
	    tmp = cell_alloc();
	    tmp->w[1].p = stack;
	    tmp->w[3].p = arg;
	    stack = tmp;
	    break;
	default:
	    if (search_env(sip, env, &arg)) {
		cell_retain(arg);
	    } else {
		tmp = cell_alloc();
		tmp->w[1].p = &thunk_src;
		tmp->w[2].p = NULL;
		tmp->w[3].p = sip;
		arg = cell_alloc();
		arg->w[1].p = &thunk_error;
		arg->w[2].p = NULL;
		arg->w[3].p = tmp;
	    }
	    skip_stem(&sip);
	    goto push;
	}
    }
  end:
    *stack_p = stack;
    *env_p = env;
    *sip_p = sip;
    return rtn;
}
static void fill_argarray(size_t length, cell * args, cell ** array)
{
    while (0 < length--) {
	*array++ = args->w[3].p;
	args = args->w[1].p;
    }
}
static void *rtn_prim(size_t length, cell ** array, cell * stack,
		      cell * continuation, cell * args)
{
    while (0 < length--) {
	cell *tos = cell_alloc();
	tos->w[1].p = stack;
	tos->w[3].p = *array++;
	stack = tos;
    }
    cell_retain(continuation);
    args_release(args);
    return beta(stack, continuation);
}
static void *beta_prim_nop(cell * stack, cell * args, word * pip)
{
    cell *cont;
    fill_argarray(1, args, &cont);
    return rtn_prim(0, NULL, stack, cont, args);
}
static const word prim_nop[] = { beta_prim_thunk, beta_prim_nop };
static void *beta_src(cell * stack, word w2, word w3)
{
    cell *env = w2.p;
    const char *sip = w3.p;
    if (exec_src(&stack, &env, &sip)) {
	w3.p = sip;
	w2.p = exec_src_closure_env(env, &sip);
	env_release(env);
	return beta_end(&thunk_src, w2, w3);
    } else {
	env_release(env);
	if (NULL != stack) {
	    cell *tos = stack, *tmp = tos->w[3].p;
	    stack = tos->w[1].p;
	    cell_free(tos);
	    return beta(stack, tmp);
	} else {
	    w2.p = NULL;
	    w3.p = prim_nop;
	    return beta_end(&thunk_prim, w2, w3);
	}
    }
}
static void thunk_src_copy(word w2, word w3)
{
    env_retain(w2.p);
}
static void thunk_no_copy(word w2, word w3)
{
    return;
}
static void *force_src(const thunk_type * tht, word w2, word w3)
{
    if ('=' == *((const char *) w3.p)) {
	thunk_src_copy(w2, w3);
	return force_end(&thunk_src, w2, w3);
    } else
	return beta_src(NULL, w2, w3);
}
static void *force_other(const thunk_type * tht, word w2, word w3)
{
    tht->copy(w2, w3);
    return force_end(tht, w2, w3);
}
static void thunk_prim_copy(word w2, word w3)
{
    args_retain(w2.p);
}
static void *beta_error(cell * stack, word w2, word w3)
{
    if (NULL != stack)
	return beta_error_core(stack, force_regfile.tht, w2, w3);
    else
	return beta_end_core(force_regfile.tht, w2, w3);
}
static void thunk_error_copy(word w2, word w3)
{
    args_retain(w2.p);
    cell_retain(w3.p);
}
static const thunk_type thunk_src =
    { force_src, beta_src, thunk_src_copy, thunk_src_release },
    thunk_prim =
    { force_other, beta_prim, thunk_prim_copy, thunk_prim_release },
    thunk_error =
    { force_other, beta_error, thunk_error_copy, thunk_error_release };
typedef struct {
    const char *name;
    const word *priminst;
} prim_env_member;
/* #include <embed.c> */
static const thunk_type thunk_int =
    { force_other, beta_error, thunk_no_copy, thunk_nop_release },
    thunk_world = thunk_int;
static void *beta_prim_binaryopint(cell * stack, cell * args, word * pip)
{
    cell *argarray[3], *rtnv = cell_alloc();
    fill_argarray(3, args, argarray);
    rtnv->w[1].p = &thunk_int;
    rtnv->w[2].i = ((int (*)(int, int)) (pip + 1)->p)
	(argarray[0]->w[2].i, argarray[1]->w[2].i);
    return rtn_prim(1, &rtnv, stack, argarray[2], args);
}
#define BINARYOPINT(OP,name) \
static int prim_op_##name(int p, int a)\
{\
    return p OP a;\
}\
static const word prim_##name[] = {\
    beta_prim_thunk,\
    beta_prim_blob, &thunk_int, beta_prim_blob, &thunk_int,\
    beta_prim_binaryopint, prim_op_##name\
}
BINARYOPINT(+, plus);
BINARYOPINT(-, minus);
BINARYOPINT(*, mul);
static void *beta_prim_divmod(cell * stack, cell * args, word * pip)
{
    cell *argarray[3], *div = cell_alloc(), *mod =
	cell_alloc(), *rtnarray[2];
    int pas, act;
    fill_argarray(3, args, argarray);
    pas = argarray[0]->w[2].i;
    act = argarray[1]->w[2].i;
    div->w[1].p = mod->w[1].p = &thunk_int;
    div->w[2].i = pas / act;
    mod->w[2].i = pas % act;
    rtnarray[0] = mod;
    rtnarray[1] = div;
    return rtn_prim(2, rtnarray, stack, argarray[2], args);
}
static const word prim_divmod[] = {
    beta_prim_thunk,
    beta_prim_blob, &thunk_int, beta_prim_blob, &thunk_int,
    beta_prim_divmod
};
static void *beta_prim_const_int(cell * stack, cell * args, word * pip)
{
    cell *conti, *rtnv = cell_alloc();
    fill_argarray(1, args, &conti);
    rtnv->w[1].p = &thunk_int;
    rtnv->w[2].i = (pip + 1)->p;
    return rtn_prim(1, &rtnv, stack, conti, args);
}
#define CONSTINT(value,name) \
static const word prim_##name[] =\
    { beta_prim_thunk, beta_prim_const_int, value }
CONSTINT(0, zero);
CONSTINT(1, one);
CONSTINT(EOF, eof);
static void *beta_prim_compareint(cell * stack, cell * args, word * pip)
{
    cell *argarray[4], *conti;
    fill_argarray(4, args, argarray);
    if (((int (*)(int, int)) (pip + 1)->p)
	(argarray[0]->w[2].i, argarray[1]->w[2].i))
	conti = argarray[3];
    else
	conti = argarray[2];
    return rtn_prim(0, NULL, stack, conti, args);
}
#define COMPAREINT(OP,name) \
static int prim_op_##name(int tgt, int std)\
{\
    return tgt OP std;\
}\
static const word prim_##name[] = {\
    beta_prim_thunk, beta_prim_thunk,\
    beta_prim_blob, &thunk_int, beta_prim_blob, &thunk_int,\
    beta_prim_compareint, prim_op_##name\
}
COMPAREINT( ==, equal);
COMPAREINT(>, big);
COMPAREINT(>=, eqbig);
static void *beta_prim_startworld(cell * stack, cell * args, cell * pip)
{
    cell *conti, *rtnv = cell_alloc();
    fill_argarray(1, args, &conti);
    rtnv->w[1].p = &thunk_world;
    return rtn_prim(1, &rtnv, stack, conti, args);
}
static const word prim_startworld[] =
    { beta_prim_thunk, beta_prim_startworld };
static void *beta_prim_forkworld(cell * stack, cell * args, word * pip)
{
    int i;
    cell *argarray[2], *rtnarray[2];
    fill_argarray(2, args, argarray);
    for (i = 0; i < 2; i++) {
	cell *cur = cell_alloc();
	cur->w[1].p = &thunk_world;
	rtnarray[i] = cur;
    }
    return rtn_prim(2, rtnarray, stack, argarray[1], args);
}
static const word prim_forkworld[] = {
    beta_prim_thunk, beta_prim_blob, &thunk_world, beta_prim_forkworld
};
static void *beta_prim_joinworld(cell * stack, cell * args, cell * pip)
{
    cell *argarray[3], *rtnv = cell_alloc();
    fill_argarray(3, args, argarray);
    rtnv->w[1].p = &thunk_world;
    return rtn_prim(1, &rtnv, stack, argarray[2], args);
}
static const word prim_joinworld[] = {
    beta_prim_thunk,
    beta_prim_blob, &thunk_world, beta_prim_blob, &thunk_world,
    beta_prim_joinworld
};
static void *beta_prim_putc_core(cell * stack, cell * args, word * pip)
{
    cell *argarray[3], *rtnv = cell_alloc();
    fill_argarray(3, args, argarray);
    rtnv->w[1].p = &thunk_world;
    fputc(argarray[1]->w[2].i, ((FILE * (*)(void)) (pip + 1)->p) ());
    return rtn_prim(1, &rtnv, stack, argarray[2], args);
}
#define PUTC_CORE(file,name) \
static FILE *prim_op_##name(void)\
{\
    return file;\
}\
static const word prim_##name[] = {\
    beta_prim_thunk,\
    beta_prim_blob, &thunk_int, beta_prim_blob, &thunk_world,\
    beta_prim_putc_core, prim_op_##name\
}
PUTC_CORE(stdout, putc);
PUTC_CORE(stderr, errc);
static void *beta_prim_getc(cell * stack, cell * args, word * pip)
{
    cell *argarray[2], *rtnarray[2];
    fill_argarray(2, args, argarray);
    rtnarray[0] = cell_alloc();
    rtnarray[0]->w[1].p = &thunk_world;
    rtnarray[1] = cell_alloc();
    rtnarray[1]->w[1].p = &thunk_int;
    rtnarray[1]->w[2].i = fgetc(stdin);
    return rtn_prim(2, rtnarray, stack, argarray[1], args);
}
static const word prim_getc[] = {
    beta_prim_thunk, beta_prim_blob, &thunk_world, beta_prim_getc
};
#define PRIMITIVE(name) { #name "=", prim_##name }
static const prim_env_member primitives[] = {
    PRIMITIVE(plus), PRIMITIVE(minus), PRIMITIVE(mul), PRIMITIVE(divmod),
    PRIMITIVE(zero), PRIMITIVE(one), PRIMITIVE(eof),
    PRIMITIVE(equal), PRIMITIVE(big), PRIMITIVE(eqbig),
    PRIMITIVE(startworld), PRIMITIVE(forkworld), PRIMITIVE(joinworld),
    PRIMITIVE(putc),
#ifndef ALLOC_TRACE
    PRIMITIVE(errc),
#endif
    PRIMITIVE(getc)
};
#include "romsrc.c"
/* embed.c end */
static cell *gen_prim_env(size_t nmemb, const prim_env_member * membs)
{
    cell *rtn = NULL;
    while (0 < nmemb) {
	cell *tmp = cell_alloc(), *arg = cell_alloc();
	arg->w[1].p = &thunk_prim;
	arg->w[2].p = NULL;
	arg->w[3].p = membs->priminst;
	tmp->w[1].p = rtn;
	tmp->w[2].p = membs->name;
	tmp->w[3].p = arg;
	rtn = tmp;
	nmemb--;
	membs++;
    }
    return rtn;
}
static int place_source(const char *path, const char **sip_p)
{
    cell *cur = ram;
    FILE *file = fopen(path, "r");
    const char *sip = ram->w[0].m;
    size_t brac = 0;
    while (1) {
	size_t i, byte = fread(cur, 1, sizeof(cell), file);
	for (i = 0; i < byte; i++)
	    switch (cur->w[0].m[i]) {
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
	    cur->w[0].m[byte] = ')';
	    break;
	}
	cur++;
    }
    fclose(file);
    if (0 < brac)
	return 2;
    cell_unused = cur + 1;
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
    int rst;
    const char *src;
    cell top, *stack = NULL;
    if (2 != argc) {
	fprintf(stderr, "%s [source file]\n", argv[0]);
	return 1;
    }
    rst = place_source(argv[1], &src);
    if (rst)
	return rst + 1;
    top.w[0].s = 1;
    top.w[1].p = &thunk_src;
    top.w[2].p =
	gen_prim_env(sizeof(primitives) / sizeof(prim_env_member),
		     primitives);
    top.w[3].p = romsrc;
    rst = exec_src(&stack, &top.w[2].p, &top.w[3].p);
    assert(exec_src_nop == rst && NULL == stack);
    top.w[3].p = src;
    force(&top);
    return !(0 == top.w[0].s && &thunk_world == top.w[1].p);
}
