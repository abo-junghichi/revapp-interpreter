#include <unistd.h>
#include <sys/syscall.h>
#include "system.h"
#define Syscall_core "syscall\n\t"
#define Clobber "memory", "rcx", "r11"
static long syscall1(long syscall, long arg1)
{
    long ret;
    __asm__ volatile (Syscall_core:"=a"(ret):"a"(syscall),
		      "D"(arg1):Clobber);
    return ret;
}
static long syscall2(long syscall, long arg1, long arg2)
{
    long ret;
    __asm__ volatile (Syscall_core:"=a"(ret):"a"(syscall), "D"(arg1),
		      "S"(arg2):Clobber);
    return ret;
}
static long syscall3(long syscall, long arg1, long arg2, long arg3)
{
    long ret;
    __asm__ volatile (Syscall_core:"=a"(ret):"a"(syscall), "D"(arg1),
		      "S"(arg2), "d"(arg3):Clobber);
    return ret;
}
size_t my_read(int fd, void *buf, size_t count)
{
    char *buf_start = buf, *buf_done = buf;
    while (0 < count) {
	int rst = syscall3(SYS_read, fd, (long) buf_done, count);
	if (0 >= rst)
	    break;
	buf_done += rst;
	count -= rst;
    }
    return buf_done - buf_start;
}
void my_write(int fd, const void *buf, size_t count)
{
    syscall3(SYS_write, fd, (long) buf, count);
}
void my_exit(int status)
{
    syscall1(__NR_exit, status);
}
int my_open(const char *pathname)
{
    int flags = 0;
    return syscall2(__NR_open, (long) pathname, flags);
}
int my_close(int fd)
{
    return syscall1(__NR_close, fd);
}
extern int main(int argc, char **argv);
void _start_main(char **ebp)
{
    int argc;
    char **argv;
    argc = (int) ebp[0];
    argv = ebp + 1;
    my_exit(main(argc, argv));
}
asm(".globl _start\n" ".type _start, @function\n" "_start:\n"
    "movq %rsp, %rdi\n" "call _start_main\n" ".size _start, .-_start\n");
