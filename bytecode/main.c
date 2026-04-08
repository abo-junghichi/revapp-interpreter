#include "system.c"
#include "revappb.c"
/*
thunk =word:tht [
| prim[argc addr rtn_enum]
| (gen_const)[??????]
| bracket
| src(|done)[cell_share ip env]
| (blob)[cell_share ?????]
]
*/
#include "primitives.c"
static int check_exit(int rst, force_state fs)
{
    return 1 != rst || NULL != fs.forcing || &thunk_world != fs.tht
	|| 0 != fs.reg_used || NULL != fs.stack_tail || 0 != fs.sc_used;
}
/*
static size_t load_regs_ro(word *regfile, cell *cep)
{
    size_t regc = 0;
    cell *tail = cep->w[3].p;
    while (NULL != tail) {
	int i;
	cell *cur = tail;
	for (i = 0; i < 3; i++) {
	    word *thp = cur->w[i].p;
	    if (NULL == thp)
		goto end;
	    regfile[regc++].p = thp;
	}
	tail = cur->w[3].p;
    }
  end:
    return regc;
}
static int verb_check_cell(word *regfile, size_t *reg_used)
{
    word *tgt = regfile[0].p;
    if (&thunk_src_done == tgt[0].c || &thunk_src == tgt[0].c) {
	cell *cep = (cell *) tgt;
	size_t i, regc;
	word buf[999];
	fprintf(stderr, "tht = %p, ip = %p\n", cep->w[0].c, cep->w[2].c);
	regc = load_regs_ro(buf, cep);
	for (i = 0; i < regc; i++)
	    fprintf(stderr, " %p", buf[i]);
	fprintf(stderr, "\n");
    } else
	fprintf(stderr, "not src[_done]\n");
    return 0;
}
static const word embed_check_cell[] = {
    { (intptr_t) & thunk_verb }, { 1 }, { (intptr_t) & verb_check_cell },
    { 1 }
};
static void print_error(const char *msg)
{
    size_t msg_len;
    for (msg_len = 0; '\0' != msg[msg_len]; msg_len++);
    my_write(my_stderr, msg, msg_len);
}
*/
#include "application.c"
#define RAM_SIZE 0xffffff
static cell ram[RAM_SIZE];
static cell *runout(void)
{
    my_exit(42);
}
int main(void)
{
    word stack_cache[STACK_CACHE_SIZE];
    force_state fs;
    int rst;
    unsigned int maxregc;
    if (maxregc_application > maxregc_primitives)
	maxregc = maxregc_application;
    else
	maxregc = maxregc_primitives;
    cell_allocator_init(ram + RAM_SIZE, ram + (maxregc + 3) / 4, runout);
    fs.forcing = NULL;
    fs.tht = NULL;
    fs.stack_tail = NULL;
    fs.sc_used = 0;
    fs.reg_used = 0;
    fs.w.c = embed_application;
    rst = force(stack_cache, ram[0].w, &fs);
    if (!check_exit(rst, fs))
	return 0;
    return 1;
}
