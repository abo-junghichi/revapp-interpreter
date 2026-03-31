#include <stdio.h>
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
/*
static int consign(word *regfile, const unsigned char *bytecode,
		   force_state *fs)
{
    size_t reg_used_org = fs->reg_used, reg_used_new;
    uintptr_t i, entrypoint = 0;
    fs->ip.c = bytecode;
    if (shiftin_core(&fs->ip, (intptr_t *) & entrypoint))
	return 1;
    while (1) {
	uintptr_t peek = 0;
	cell *tmp;
	if (shiftin_core(&fs->ip, (intptr_t *) & peek))
	    break;
	if (peek >= reg_used_org)
	    return 1;
	tmp = regfile[peek].p;
	(*cell_share(tmp))++;
	regfile[fs->reg_used++].p = tmp;
    }
    for (i = 0; i < reg_used_org; i++)
	thunk_release(regfile[i].p);
    reg_used_new = fs->reg_used - reg_used_org;
    for (i = 0; i < reg_used_new; i++)
	regfile[i] = regfile[i + reg_used_org];
    fs->reg_used = reg_used_new;
    fs->ip.c = ((const unsigned char *) fs->ip.p) + entrypoint;
    return 0;
}
*/
#include "primitives.c"
static int check_exit(int rst, force_state fs)
{
    return 1 != rst || NULL != fs.forcing || &thunk_world != fs.tht
	|| 0 != fs.reg_used || NULL != fs.stack_tail || 0 != fs.sc_used;
}
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
/* application */
#include "application.c"
/*
static const unsigned char bytecode_application[] = {
    // [s]((=w)= s s)=a -> 0 0 .
    0xb0, 0xb0, 0x02,
    // [putc(1) 42(0)](=s =w (a[s]) w putc 42)=b ->
    // =2(3) =2
    0x92, 0x92,
    // yet([a] 3)
    0xdb, 0x83, 0x00,
    // 2 1 0 .
    0xb2, 0xb1, 0xb0, 0x02,
    // [putc(2) 42(1) world(0)]((b[putc 42]) =s  world s s) ->
    // done([b] 1 2) rl(2)
    0xf7, 0x81, 0x82, 0x00, 0x72,
    // =0 1 0 0 .
    0x90, 0xb1, 0xb0, 0xb0, 0x02
};
static const word embed_application[] = {
    { (intptr_t) & thunk_src_const },
    { (intptr_t) bytecode_application + 12 },
    { (intptr_t) embed_world }, { (intptr_t) embed_const_42 },
    { (intptr_t) embed_putc }, { 0 }
};
*/
/*
static const unsigned char bytecode_application[] = {
    // bracket ref(0) ref(2) ref(1) cket
    0x03, 0xb0, 0xb2, 0xb1, 0x02
};
static const word embed_application[] = {
    { (intptr_t) & thunk_src_const },
    { (intptr_t) bytecode_application + 0 },
    { (intptr_t) embed_world }, { (intptr_t) embed_const_42 },
    { (intptr_t) embed_putc }, { 0 }
};
*/
/* end of application */
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
    cell_allocator_init(ram + RAM_SIZE,
			ram + (maxregc_application + 3) / 4, runout);
    fs.forcing = NULL;
    fs.tht = NULL;
    fs.stack_tail = NULL;
    fs.sc_used = 0;
    fs.reg_used = 0;
    fs.w.c = embed_application;
    rst = force(stack_cache, ram[0].w, &fs);
    if (!check_exit(rst, fs))
	return 0;
    printf("rst = %i\n", rst);
    printf("forcing = %p\n", fs.forcing);
    printf("tht = %p\n", fs.tht);
#define print_tht(stem) printf("%p = thunk_" #stem "\n", &thunk_##stem)
    print_tht(src);
    print_tht(src_done);
    print_tht(src_const);
    print_tht(verb);
    print_tht(world);
    print_tht(int);
    print_tht(gen_int);
    printf("reg_used = %zu\n", fs.reg_used);
    printf("sc_used = %u, tail = %p\n", fs.sc_used, fs.stack_tail);
    printf("pointer %p\n", embed_putc);
    printf("ip = %p, bytecode = %p\n", fs.w.p, bytecode_application);
    return 1;
}
static int load_bytecode_raw(const char *path)
{
    int rtn = 0;
    int fd = my_open(path);
    char *cip = ram[0].m;
    while (1) {
	char peek;
	if (1 != my_read(fd, &peek, 1)) {
	    rtn = 1;
	    goto end;
	}
	if ('\n' == peek)
	    break;
    }
    cip += my_read(fd, cip, sizeof(ram));
    cell_limit =
	ram + (size_t) (cip - ram[0].m + sizeof(cell) - 1) / sizeof(cell);
  end:
    my_close(fd);
    return rtn;
}
static void dump_bytecode(void)
{
    my_write(my_stderr, ram, sizeof(cell) * (cell_limit - ram));
}
static void print_error(const char *msg)
{
    size_t msg_len;
    for (msg_len = 0; '\0' != msg[msg_len]; msg_len++);
    my_write(my_stderr, msg, msg_len);
}
/*
int main(int argc, char **argv)
{
    bit_cache bc;
    if (2 != argc) {
	print_error(argv[0]);
	print_error(" [bytecode file]\n");
	return 1;
    }
    bc = bit_cache_init(argv[1]);
    while (1) {
	int rst = read_bit(&bc);
	if (rst < 0)
	    return 0;
	else {
	    char c = rst + '0';
	    my_write(my_stdout, &c, 1);
	}
    }
}
*/
