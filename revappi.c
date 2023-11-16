#include <assert.h>
#ifdef ALLOC_TRACE
#include <stdio.h>
#endif
#include "revappi.h"
static cell *free_cell =
    NULL, *cell_spare, *cell_limit, *(*cell_runout)(void);
void cell_allocator_init(cell *start, cell *end, cell *(*runout) (void))
{
    cell_spare = start;
    cell_limit = end;
    cell_runout = runout;
}
cell *cell_alloc(void)
{
    cell *rtn = free_cell;
    if (rtn)
	free_cell = rtn->free;
    else if (cell_limit > cell_spare)
	rtn = cell_spare++;
    else
	rtn = cell_runout();
#ifdef ALLOC_TRACE
    fprintf(stderr, "@ a.out + %p 0x%x\n", (void *) rtn, sizeof(cell));
#endif
    return rtn;
}
void cell_free(void *cellp)
{
    cell *cp = cellp;
    cp->free = free_cell;
    free_cell = cp;
#ifdef ALLOC_TRACE
    fprintf(stderr, "@ a.out - %p\n", (void *) cp);
#endif
}
static thunk *thunk_alloc(void)
{
    thunk *rtn = &cell_alloc()->th;
    rtn->share = 0;
    return rtn;
}
static list *list_alloc_tail(list *tail)
{
    list *rtn = &cell_alloc()->li;
    rtn->share = 0;
    rtn->tail = tail;
    return rtn;
}
static list *list_alloc(list *tail, thunk *thp)
{
    list *rtn = list_alloc_tail(tail);
    rtn->u.thp = thp;
    return rtn;
}
static void list_retain(list *lp)
{
    if (lp)
	lp->share++;
}
static void list_release(list *lp)
{
    list *prev = NULL;
    thunk *thp;
    list *tail;
  list_entry:
    if (!lp)
	goto list_end;
    if (0 < lp->share) {
	lp->share--;
	goto list_end;
    }
    thp = lp->u.thp;
    if (0 < thp->share)
	thp->share--;
    else {
	list *next = thp->tht->release(thp->c);
	cell_free(thp);
	if (next) {
	    lp->u.gc_prev = prev;
	    prev = lp;
	    lp = next;
	    goto list_entry;
	}
    }
  free_self:
    tail = lp->tail;
    cell_free(lp);
    lp = tail;
    goto list_entry;
  list_end:
    if (prev) {
	lp = prev;
	prev = lp->u.gc_prev;
	goto free_self;
    }
    return;
}
static beta_rst beta_src(force_regfile *);
static void thunk_src_retain(thunk_cont cont)
{
    list_retain(cont.src.env);
}
static list *thunk_src_release(thunk_cont cont)
{
    return cont.src.env;
}
#define THUNK_TYPE_SRC(postfix) \
static const thunk_type thunk_src##postfix =\
    { beta_src, thunk_src_retain, thunk_src_release }
THUNK_TYPE_SRC();
THUNK_TYPE_SRC(_done);
beta_rst beta_undefined(force_regfile *r)
{
    return beta_error;
}
static const thunk_type thunk_error =
    { beta_undefined, thunk_src_retain, thunk_src_release };
static beta_rst beta_prim(force_regfile *);
static void thunk_prim_retain(thunk_cont cont)
{
    list_retain(cont.prim.args);
}
static list *thunk_prim_release(thunk_cont cont)
{
    return cont.prim.args;
}
static const thunk_type thunk_prim =
    { beta_prim, thunk_prim_retain, thunk_prim_release };
static thunk *force(force_regfile *r, thunk *thp)
{
    thunk *tmp;
    r->tht = thp->tht;
    r->thc = thp->c;
    thp->c.force.stack = NULL;
    thp->c.force.thp = NULL;
    r->stack = NULL;
    while (1)
	switch (r->tht->beta(r)) {
	case beta_force:
	    tmp = r->thc.force.thp;
	    r->tht = tmp->tht;
	    r->thc = tmp->c;
	    if (0 >= tmp->share)
		cell_free(tmp);
	    else {
		tmp->share--;
		if (&thunk_src != r->tht)
		    r->tht->retain(r->thc);
		else {
		    tmp->c.force.stack = r->stack;
		    r->stack = NULL;
		    tmp->c.force.thp = thp;
		    thp = tmp;
		}
	    }
	    break;
	case beta_done:
	    tmp = thp;
	    thp = tmp->c.force.thp;
	    r->stack = tmp->c.force.stack;
	    tmp->tht = r->tht;
	    tmp->c = r->thc;
	    r->tht->retain(r->thc);
	    if (thp)
		break;
	case beta_error:
	default:
	    return thp;
	}
}
#define WHITESPACE ' ': case '\n': case '\t'
static int is_delimiter(const char c)
{
    switch (c) {
    case WHITESPACE:
    case '=':
    case '(':
    case ')':
	return 1;
    default:
	return 0;
    }
}
static void skip_stem(const char **sip)
{
    while (!is_delimiter(*++*sip));
}
static int search_env(const char *sip, list *env, thunk **found)
{
    while (env) {
	const char *std = sip, *tgt = env->name;
	while (!is_delimiter(*std))
	    if (*(std++) != *(tgt++))
		goto next;
	if (is_delimiter(*tgt)) {
	    *found = env->u.thp;
	    return 1;
	}
      next:
	env = env->tail;
    }
    return 0;
}
static list *exec_src_closure_env(list *env_parent, const char **sip)
{
    size_t depth = 0;
    list *env = NULL, *mask = NULL;
    while (1) {
	thunk *thp;
	switch (**sip) {
	case WHITESPACE:
	    (*sip)++;
	    break;
	case '=':
	    mask = list_alloc_tail(mask);
	    mask->share = depth;
	    mask->name = *sip + 1;
	    skip_stem(sip);
	    break;
	case ')':
	    while (mask && depth <= mask->share) {
		list *tmp = mask->tail;
		cell_free(mask);
		mask = tmp;
	    }
	    (*sip)++;
	    if (0 >= depth)
		return env;
	    depth--;
	    break;
	case '(':
	    (*sip)++;
	    depth++;
	    break;
	default:
	    if (!search_env(*sip, env, &thp)
		&& !search_env(*sip, mask, &thp)
		&& search_env(*sip, env_parent, &thp)) {
		thp->share++;
		env = list_alloc(env, thp);
		env->name = *sip;
	    }
	    skip_stem(sip);
	    break;
	}
    }
}
typedef enum { exec_src_nop = 0, exec_src_func } exec_src_rst;
static exec_src_rst exec_src(list **stack, list **env, const char **sip)
{
    while (1) {
	list *tmp;
	thunk *arg;
	switch (**sip) {
	case WHITESPACE:
	    (*sip)++;
	    break;
	case '=':
	    if (*stack) {
		tmp = *stack;
		*stack = tmp->tail;
		tmp->tail = *env;
		tmp->name = *sip + 1;
		*env = tmp;
		skip_stem(sip);
		break;
	    } else
		return exec_src_func;
	case ')':
	    return exec_src_nop;
	case '(':
	    arg = thunk_alloc();
	    arg->tht = &thunk_src;
	    arg->c.src.sip = ++*sip;
	    arg->c.src.env = exec_src_closure_env(*env, sip);
	  push:
	    *stack = list_alloc(*stack, arg);
	    break;
	default:
	    if (search_env(*sip, *env, &arg))
		arg->share++;
	    else {
		arg = thunk_alloc();
		arg->tht = &thunk_error;
		arg->c.src.sip = *sip;
		arg->c.src.env = *env;
		(*env)->share++;
	    }
	    skip_stem(sip);
	    goto push;
	}
    }
}
static size_t beta_prim_nop(word *regfile)
{
    return 0;
}
static const thunk_type *const dist_nop_0[] = { NULL };
static const prim_src prim_nop[] =
    { PIP_POP_THUNK, PIP_CALL(nop), PIP_DIST(nop_0) };
static beta_rst beta_src(force_regfile *r)
{
    list *env = r->thc.src.env;
    beta_rst rtn;
    if (exec_src(&r->stack, &env, &r->thc.src.sip)) {
	const char *sip = r->thc.src.sip;
	r->thc.src.env = exec_src_closure_env(env, &sip);
	r->tht = &thunk_src_done;
	rtn = beta_done;
    } else if (r->stack) {
	list *tmp = r->stack;
	r->thc.force.thp = tmp->u.thp;
	r->stack = tmp->tail;
	cell_free(tmp);
	rtn = beta_force;
    } else {
	r->tht = &thunk_prim;
	r->thc.prim.args = NULL;
	r->thc.prim.pip = prim_nop;
	rtn = beta_done;
    }
    list_release(env);
    return rtn;
}
static beta_rst beta_prim(force_regfile *r)
{
    int i;
    word prim_regfile[PRIM_REGFILE_SIZE];
    list *tmp;
    const prim_src *pip;
    const thunk_type *const *dist;
    while (!r->thc.prim.pip->func) {
	const thunk_type *tht;
	tmp = r->stack;
	if (NULL == tmp)
	    return beta_done;
	tht = r->thc.prim.pip->pointer;
	if (NULL != tht && tht != tmp->u.thp->tht)
	    return beta_error;
	r->stack = tmp->tail;
	tmp->tail = r->thc.prim.args;
	r->thc.prim.args = tmp;
	r->thc.prim.pip++;
    }
    pip = r->thc.prim.pip;
    tmp = r->thc.prim.args;
    i = 0;
    while (1) {
	if (NULL == tmp)
	    break;
	pip--;
	if (NULL != pip->pointer)
	    prim_regfile[i] = tmp->u.thp->c.blob;
	else
	    prim_regfile[i].thp = tmp->u.thp;
	i++;
	tmp = tmp->tail;
    }
    pip = r->thc.prim.pip;
    dist = (pip + 1 + pip->func(prim_regfile))->pointer;
    i = 0;
    while (1) {
	const thunk_type *tht = *dist;
	thunk *thp;
	if (!tht)
	    break;
	thp = thunk_alloc();
	thp->tht = tht;
	thp->c.blob = prim_regfile[i++];
	r->stack = list_alloc(r->stack, thp);
	dist++;
    }
    tmp = r->thc.prim.args;
    r->thc.force.thp = prim_regfile[i].thp;
    r->thc.force.thp->share++;
    list_release(tmp);
    return beta_force;
}
void thunk_nop_retain(thunk_cont c)
{
    /* return; */
}
list *thunk_nop_release(thunk_cont c)
{
    return NULL;
}
const thunk_type thunk_world =
    { beta_undefined, thunk_nop_retain, thunk_nop_release };
static list *gen_prim_env(const prim_env_member *membs)
{
    list *rtn = NULL;
    while (NULL != membs->pip) {
	thunk *arg = thunk_alloc();
	arg->tht = &thunk_prim;
	arg->c.prim.args = NULL;
	arg->c.prim.pip = membs->pip;
	rtn = list_alloc(rtn, arg);
	rtn->name = membs->name;
	membs++;
    }
    return rtn;
}
int revapp_interp(const prim_env_member *membs, const char *romsrc,
		  const char *ramsrc)
{
    exec_src_rst rst;
    thunk top, *thp;
    force_regfile r;
    list *stack = NULL;
    top.tht = &thunk_src;
    top.c.src.env = gen_prim_env(membs);
    top.c.src.sip = romsrc;
    rst = exec_src(&stack, &top.c.src.env, &top.c.src.sip);
    assert(exec_src_nop == rst && NULL == stack);
    top.c.src.sip = ramsrc;
    thp = force(&r, &top);
    return !(&top == thp && NULL == top.c.force.thp
	     && NULL == top.c.force.stack && NULL == r.stack
	     && &thunk_world == r.tht);
}
