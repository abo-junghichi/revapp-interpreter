#include <stddef.h>
#define PRIM_REGFILE_SIZE 10
typedef struct thunk thunk;
typedef union {
    thunk *thp;
    int i;
} word;
typedef struct {
    /*
       match (pointer,func) with
       (NULL,NULL) -> pop thunk
       (&thunk_type,NULL) -> pop blob
       (*,func) -> call func
       (&thunk_type[],*) -> dist
     */
    const void *pointer;
    size_t (*func)(word *);
} prim_src;
typedef struct list list;
struct list {
    size_t share;
    list *tail;
    union {
	list *gc_prev;
	thunk *thp;
    } u;
    const char *name;
};
typedef union {
    word blob;
    struct {
	list *env;
	const char *sip;
    } src;
    struct {
	list *args;
	const prim_src *pip;
    } prim;
    struct {
	thunk *thp;
	list *stack;
    } force;
} thunk_cont;
typedef struct thunk_type thunk_type;
typedef struct {
    list *stack;
    const thunk_type *tht;
    thunk_cont thc;
} force_regfile;
typedef enum { beta_force, beta_done, beta_error } beta_rst;
struct thunk_type {
    beta_rst(*beta) (force_regfile *);
    void (*copy)(thunk_cont);
    list *(*release)(thunk_cont);
};
struct thunk {
    size_t share;
    const thunk_type *tht;
    thunk_cont c;
};
#define CELL_CONT thunk th; list li; cell *free
#define CELL_SIZE sizeof(union { CELL_CONT; })
typedef union cell cell;
union cell {
    CELL_CONT;
    char m[CELL_SIZE];
};
cell *cell_alloc(void);
void cell_free(void *);
#define PIP_POP(thunk_type_stem) {&thunk_##thunk_type_stem,NULL}
#define PIP_DIST(dist_stem) {dist_##dist_stem,NULL}
#define PIP_POP_THUNK {NULL,NULL}
#define PIP_CALL(func_stem) {NULL,beta_prim_##func_stem}
beta_rst beta_undefined(force_regfile *);
void thunk_nop_copy(thunk_cont);
list *thunk_nop_release(thunk_cont);
extern const thunk_type thunk_world;
typedef struct {
    const char *name;
    const prim_src *pip;
} prim_env_member;
#define PRIMITIVE(name) { #name "=", prim_##name }
#define PRIMITIVE_END { NULL, NULL }
int main_core(const prim_env_member * membs, const char *romsrc, int argc,
	      char **argv);
