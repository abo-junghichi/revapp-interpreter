#ifndef REVAPPB_C
#define REVAPPB_C
#ifdef ALLOC_TRACE
#include <stdio.h>
#endif
#include <stddef.h>
#include <stdint.h>
#define WORD_SIZE sizeof(intptr_t)
typedef union {
    intptr_t i;
    void *p;
    const void *c;
    char m[WORD_SIZE];
} word;
#define CELL_CONT word w[4]
#define CELL_SIZE sizeof(union { CELL_CONT; })
typedef union cell cell;
union cell {
    CELL_CONT;
    char m[CELL_SIZE];
};
static cell *free_cell, *cell_spare, *cell_limit, *(*cell_runout)(void);
static void cell_allocator_init(cell *start, cell *end,
				cell *(*runout) (void))
{
    free_cell = NULL;
    cell_spare = start;
    cell_limit = end;
    cell_runout = runout;
}
static cell *cell_alloc(void)
{
    cell *rtn = free_cell;
    if (rtn)
	free_cell = rtn->w[0].p;
    else if (cell_limit < cell_spare)
	rtn = --cell_spare;
    else
	rtn = cell_runout();
#ifdef ALLOC_TRACE
    fprintf(stderr, "@ a.out + %p 0x%x\n", (void *) rtn, sizeof(cell));
#endif
    return rtn;
}
static void cell_free(cell *cellp)
{
    cellp->w[0].p = free_cell;
    free_cell = cellp;
#ifdef ALLOC_TRACE
    fprintf(stderr, "@ a.out - %p\n", (void *) cellp);
#endif
}
typedef struct force_state force_state;
typedef struct {
    int (*beta)(word *, word *, force_state *);
    void (*retain)(word *);
    word *(*release)(word *);
    word *(*release_resume)(cell *);
} thunk_type;
static cell *thunk_release_light(cell *prev, word **thp,
				 word *(**resume) (cell *)
    )
{
    const thunk_type *tht = (*thp)->c;
    word *child = tht->release(*thp);
    if (NULL == child)
	return NULL;
    else {
	cell *rtn = (cell *) * thp;
	*thp = child;
	rtn->w[0].i = (intptr_t) (*resume = tht->release_resume);
	rtn->w[1].p = prev;
	return rtn;
    }
}
static void thunk_release(word *thp)
{
    word *(*resume)(cell *);
    cell *cep = thunk_release_light(NULL, &thp, &resume);
    if (NULL == cep)
	return;
    while (1) {
	cell *chcep = thunk_release_light(cep, &thp, &resume);
	if (NULL != chcep)
	    cep = chcep;
	else
	    while (NULL == (thp = resume(cep))) {
		chcep = cep;
		cep = chcep->w[1].p;
		cell_free(chcep);
		if (NULL == cep)
		    return;
		resume = (word * (*)(cell *)) cep->w[0].i;
	    }
    }
}
static void thunk_retain(word *thp)
{
    ((const thunk_type *) thp->c)->retain(thp);
}
static int thunk_release_cell_core(cell *cep)
{
    uintptr_t share = cep->w[1].i;
    if (0 >= share--)
	return 0;
    cep->w[1].i = share;
    return 1;
}
static void retain_cell(word *thp)
{
    cell *cep = (cell *) thp;
    cep->w[1].i++;
}
static word *release_leaf(word *thp)
{
    cell *cep = (cell *) thp;
    if (!thunk_release_cell_core(cep))
	cell_free(cep);
    return NULL;
}
static void retain_readonly(word *_)
{
    return;
}
static word *release_readonly(word *_)
{
    return NULL;
}
#define THUNK_CONST(name,beta) static const thunk_type name = { beta, retain_readonly, release_readonly, NULL }
static word *thunk_src_release_core(cell *cep, uintptr_t done_count)
{
    cell *holder = cep->w[3].p;
    word *child;
    if (NULL == holder)
	return NULL;
    child = holder->w[done_count++].p;
    if (NULL == child) {
	cell_free(holder);
	return NULL;
    }
    if (3 <= done_count) {
	cep->w[3] = holder->w[3];
	cell_free(holder);
	done_count = 0;
    }
    cep->w[2].i = done_count;
    return child;
}
static word *thunk_src_release_resume(cell *cep)
{
    return thunk_src_release_core(cep, cep->w[2].i);
}
static word *thunk_src_release(word *thp)
{
    cell *cep = (cell *) thp;
    if (thunk_release_cell_core(cep))
	return NULL;
    if (NULL == cep->w[3].p) {
	cell_free(cep);
	return NULL;
    }
    return thunk_src_release_core(cep, 0);
}
static int thunk_src_beta(word *, word *, force_state *);
#define THUNK_SRC(name) static const thunk_type name = { thunk_src_beta, retain_cell, thunk_src_release, thunk_src_release_resume }
THUNK_SRC(thunk_src);
THUNK_SRC(thunk_src_done);
THUNK_CONST(thunk_src_const, thunk_src_beta);
struct force_state {
    cell *forcing, *stack_tail;
    const thunk_type *tht;
    word w;
    size_t reg_used;
    unsigned int sc_used;
};
static int force(word *stack_cache, word *regfile, force_state *fs)
{
    while (1) {
	int rtn;
	fs->tht = ((word *) fs->w.p)->c;
	rtn = fs->tht->beta(stack_cache, regfile, fs);
	if (rtn)
	    return rtn;
    }
}
static void stack_cache_reduce(word *stack_cache, unsigned int floor,
			       force_state *fs)
{
    unsigned int i, n, pushed = 0, sc_used = fs->sc_used, topush;
    cell *st;
    if (floor + 3 > sc_used)
	return;
    topush = sc_used - floor - 3;
    st = fs->stack_tail;
    while (pushed <= topush) {
	cell *nst = cell_alloc();
	nst->w[3].p = st;
	for (i = 0; i < 3; i++)
	    nst->w[2 - i] = stack_cache[pushed + i];
	pushed += 3;
	st = nst;
    }
    fs->stack_tail = st;
    n = fs->sc_used - pushed;
    for (i = 0; i < n; i++)
	stack_cache[i] = stack_cache[i + pushed];
    fs->sc_used = n;
}
static void save_frame(word *stack_cache, force_state *fs, cell *child)
{
    cell *stt;
    child->w[0].p = fs->forcing;
    stack_cache_reduce(stack_cache, 0, fs);
    if (1 == fs->sc_used)
	child->w[2] = stack_cache[0];
    else
	child->w[2].p = NULL;
    if (2 == fs->sc_used) {
	stt = cell_alloc();
	stt->w[3].p = fs->stack_tail;
	stt->w[2] = stack_cache[0];
	stt->w[1] = stack_cache[1];
	stt->w[0].p = NULL;
    } else
	stt = fs->stack_tail;
    child->w[3].p = stt;
    fs->stack_tail = NULL;
    fs->sc_used = 0;
}
static void load_frame(word *stack_cache, cell *child, force_state *fs)
{
    cell *tail = child->w[3].p;
    word tmp = child->w[2];
    fs->forcing = child->w[0].p;
    if (NULL != tmp.p) {
	stack_cache[0] = tmp;
	fs->sc_used = 1;
	fs->stack_tail = tail;
    } else if (NULL == tail) {
	fs->sc_used = 0;
	fs->stack_tail = NULL;
    } else {
	fs->stack_tail = tail->w[3].p;
	stack_cache[0] = tail->w[2];
	stack_cache[1] = tail->w[1];
	if (NULL == (tmp.p = tail->w[0].p))
	    fs->sc_used = 2;
	else {
	    stack_cache[2] = tmp;
	    fs->sc_used = 3;
	}
	cell_free(tail);
    }
}
typedef struct {
    cell *cur;
    unsigned int wc;
} thunking;
static thunking thunking_open(cell *dist)
{
    thunking rtn;
    rtn.cur = dist;
    dist->w[3].p = NULL;
    rtn.wc = 3;
    return rtn;
}
static void thunking_write(word *thp, thunking *dist)
{
    if (3 <= dist->wc) {
	cell *cur = cell_alloc();
	dist->cur->w[3].p = cur;
	dist->cur = cur;
	dist->wc = 0;
    }
    dist->cur->w[dist->wc++].p = thp;
}
static void thunking_close(thunking *th)
{
    unsigned int wc = th->wc;
    cell *cur = th->cur;
    while (wc < 4)
	cur->w[wc++].p = NULL;
    th->cur = NULL;
}
static void save_regs(word *regfile, size_t reg_used, cell *dest)
{
    size_t i;
    thunking tmp = thunking_open(dest);
    for (i = 0; i < reg_used; i++) {
	word *thp = regfile[i].p;
	thunk_retain(thp);
	thunking_write(thp, &tmp);
    }
    thunking_close(&tmp);
}
static void load_regs(int retain_mode, word *regfile, cell *cep,
		      force_state *fs)
{
    size_t regc = 0;
    cell *tail = cep->w[3].p;
    while (NULL != tail) {
	int i;
	cell *cur = tail;
	for (i = 0; i < 3; i++) {
	    word *thp = cur->w[i].p;
	    if (NULL == thp) {
		tail = NULL;
		goto try_free;
	    }
	    regfile[regc++].p = thp;
	    if (retain_mode)
		thunk_retain(thp);
	}
	tail = cur->w[3].p;
      try_free:
	if (!retain_mode)
	    cell_free(cur);
    }
    fs->reg_used = regc;
}
static int stack_cache_pop(word *stack_cache, force_state *fs, word **ele)
{
    if (!(0 < fs->sc_used)) {
	int i;
	cell *tail = fs->stack_tail;
	if (NULL == tail)
	    return 1;
	for (i = 0; i < 3; i++)
	    stack_cache[2 - i] = tail->w[i];
	fs->stack_tail = tail->w[3].p;
	cell_free(tail);
	fs->sc_used = 3;
    }
    *ele = stack_cache[--fs->sc_used].p;
    return 0;
}
#define STACK_CACHE_SIZE (8)
static void stack_cache_push(word *stack_cache, force_state *fs, word *ele)
{
    if (fs->sc_used >= STACK_CACHE_SIZE)
	stack_cache_reduce(stack_cache, STACK_CACHE_SIZE / 2, fs);
    stack_cache[fs->sc_used++].p = ele;
}
static int shiftin_core(const unsigned char **ip, uintptr_t *rst)
{
    while (1) {
	unsigned char peek = *(*ip)++;
	uintptr_t prev = *rst;
	*rst = prev << 7 | (peek & 0x7f);
	if (peek >> 7)
	    return 0;
	else if (prev == *rst)
	    return 1;
    }
}
static int get_arg_field(const unsigned char * *ip, unsigned char inst,
			 uintptr_t *rst)
{
    *rst = *rst << 4 | (inst & 0xf);
    if (inst & 0x10)
	return 0;
    return shiftin_core(ip, rst);
}
static word *beta_brac(const thunk_type *tht, word *regfile,
		       size_t reg_used, const unsigned char *ip_org,
		       unsigned char inst, const unsigned char * *ip)
{
    intptr_t offset = -1;
    cell *rtn;
    thunking thg;
    word tmp;
    if (get_arg_field(ip, inst, (uintptr_t *) & offset))
	return NULL;
    rtn = cell_alloc();
    rtn->w[0].c = tht;
    rtn->w[1].i = 0;
    rtn->w[2].c = ip_org + offset;
    thg = thunking_open(rtn);
    while (1) {
	uintptr_t rid = 0;
	if (shiftin_core(ip, &rid))
	    break;
	if (rid >= reg_used) {
	    thunking_close(&thg);
	    thunk_release(&rtn->w[0]);
	    return NULL;
	}
	tmp = regfile[rid];
	thunk_retain(tmp.p);
	thunking_write(tmp.p, &thg);
    }
    thunking_close(&thg);
    return &rtn->w[0];
}
static int get_uarg_field(const unsigned char * *ip, unsigned char inst,
			  uintptr_t *rst)
{
    *rst = 0;
    return get_arg_field(ip, inst, rst);
}
static const unsigned char code_bracket[] = { 0x02 /*cket */  };
static const word word_bracket[] = {
    { (intptr_t) & thunk_src_const }, { (intptr_t) code_bracket },
    { (intptr_t) NULL }
};
static int thunk_src_beta(word *stack_cache, word *regfile,
			  force_state *fs)
{
    while (1) {
	word *thp = fs->w.p;
	const unsigned char *ip;
	if (&thunk_src_const == fs->tht) {
	    size_t i = 0;
	    ip = thp[1].c;
	    while (1) {
		word *cur = thp[i + 2].p;
		if (NULL == cur)
		    break;
		regfile[i++].p = cur;
	    }
	    fs->reg_used = i;
	} else {
	    cell *child = (cell *) thp;
	    ip = child->w[2].c;
	    if (0 >= (uintptr_t) child->w[1].i) {
		load_regs(0, regfile, child, fs);
		cell_free(child);
	    } else {
		child->w[1].i--;
		if (&thunk_src != fs->tht)
		    load_regs(1, regfile, child, fs);
		else {
		    load_regs(0, regfile, child, fs);
		    save_frame(stack_cache, fs, child);
		    fs->forcing = child;
		}
	    }
	}
	while (1) {
	    uintptr_t uarg;
	    const thunk_type *brac_tht;
	    const unsigned char *ip_org = ip;
	    unsigned char peek = *ip++;
	    switch (peek >> 5) {
	    case 0x0:
		switch (peek /* & 0x1f */ ) {
		case 0x1:	/* cut */
		    if (stack_cache_pop(stack_cache, fs, &thp))
			goto done;
		    thunk_release(thp);
		    break;
		case 0x2:	/* cket */
		    while (0 < fs->reg_used)
			thunk_release(regfile[--fs->reg_used].p);
		    if (stack_cache_pop(stack_cache, fs, &thp))
			goto done;
		    goto next;
		case 0x3:	/* bracket */
		    thp = word_bracket;
		    goto push_thp;
		default:
		    goto end;
		}
		break;
	    case 0x3:		/* release */
		if (get_uarg_field(&ip, peek, &uarg)
		    && uarg > fs->reg_used)
		    goto end;
		while (uarg--)
		    thunk_release(regfile[--fs->reg_used].p);
		break;
	    case 0x4:		/* def */
		if (get_uarg_field(&ip, peek, &uarg)
		    && uarg > fs->reg_used)
		    goto end;
		if (stack_cache_pop(stack_cache, fs, &thp))
		    goto done;
		regfile[fs->reg_used++] = regfile[uarg];
		regfile[uarg].p = thp;
		break;
	    case 0x5:		/* ref */
		if (get_uarg_field(&ip, peek, &uarg)
		    && uarg >= fs->reg_used)
		    goto end;
		thp = regfile[uarg].p;
		thunk_retain(thp);
		goto push_thp;
	    case 0x6:
		brac_tht = &thunk_src;
		goto do_brac;
	    case 0x7:
		brac_tht = &thunk_src_done;
		goto do_brac;
	    default:
		goto end;
	      do_brac:
		if (NULL ==
		    (thp =
		     beta_brac(brac_tht, regfile, fs->reg_used, ip_org,
			       peek, &ip)))
		    goto end;
	      push_thp:
		stack_cache_push(stack_cache, fs, thp);
		break;
	      done:
		ip = ip_org;
		if (NULL == fs->forcing)
		    goto end;
		else {
		    cell *child = fs->forcing;
		    load_frame(stack_cache, child, fs);
		    child->w[0].c = &thunk_src_done;
		    child->w[2].c = ip;
		    save_regs(regfile, fs->reg_used, child);
		}
	    }
	    continue;
	  next:
	    fs->tht = thp->c;
	    fs->w.p = thp;
	    if (thunk_src_beta != fs->tht->beta)
		return fs->tht->beta(stack_cache, regfile, fs);
	    break;
	  end:
	    fs->w.c = ip;
	    return 1;
	}
    }
}
static int thunk_verb_beta(word *stack_cache, word *regfile,
			   force_state *fs)
{
    int rst;
    word *tmp;
    const word *thp = fs->w.c;
    uintptr_t i, argc = thp[1].i, nextc;
    while (argc > fs->reg_used) {
	if (stack_cache_pop(stack_cache, fs, &tmp))
	    return 1;
	regfile[fs->reg_used++].p = tmp;
    }
    rst = ((int (*)(word *, size_t *)) thp[2].i)
	(regfile, &fs->reg_used);
    if (0 > rst)
	return rst;
    for (i = 0; i < (unsigned int) rst; i++) {
	if (stack_cache_pop(stack_cache, fs, &tmp))
	    return 1;
	thunk_release(tmp);
    }
    if (stack_cache_pop(stack_cache, fs, &tmp))
	return 1;
    fs->w.p = tmp;
    i++;
    nextc = thp[3].i;
    for (; i < nextc; i++) {
	if (stack_cache_pop(stack_cache, fs, &tmp))
	    return 1;
	else
	    thunk_release(tmp);
    }
    while (argc < fs->reg_used)
	stack_cache_push(stack_cache, fs, regfile[--fs->reg_used].p);
    while (0 < fs->reg_used)
	thunk_release(regfile[--fs->reg_used].p);
    return 0;
}
THUNK_CONST(thunk_verb, thunk_verb_beta);
static int thunk_error_beta(word *stack_cache, word *regfile,
			    force_state *fs)
{
    return 1;
}
#endif				/* REVAPPB_C */
