#include <unistd.h>
#include <sys/syscall.h>
#include "system.h"
#define Syscall_core "int $0x80"
static int syscall1(int syscall, int arg1)
{
    int ret;
    __asm__ volatile (Syscall_core:"=a"(ret):"a"(syscall),
		      "b"(arg1):"memory");
    return ret;
}
static int syscall2(int syscall, int arg1, int arg2)
{
    int ret;
    __asm__ volatile (Syscall_core:"=a"(ret):"a"(syscall), "b"(arg1),
		      "c"(arg2):"memory");
    return ret;
}
static int syscall3(int syscall, int arg1, int arg2, int arg3)
{
    int ret;
    __asm__ volatile (Syscall_core:"=a"(ret):"a"(syscall), "b"(arg1),
		      "c"(arg2), "d"(arg3):"memory");
    return ret;
}
size_t my_read(int fd, void *buf, size_t count)
{
    char *buf_start = buf, *buf_done = buf;
    while (0 < count) {
	int rst = syscall3(SYS_read, fd, (int) buf_done, count);
	if (0 >= rst)
	    break;
	buf_done += rst;
	count -= rst;
    }
    return buf_done - buf_start;
}
void my_write(int fd, const void *buf, size_t count)
{
    syscall3(SYS_write, fd, (int) buf, count);
}
void my_exit(int status)
{
    syscall1(__NR_exit, status);
}
int my_open(const char *pathname)
{
    int flags = 0;
    return syscall2(__NR_open, (int) pathname, flags);
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
asm(".globl _start\n" ".type _start, @function\n" "_start:\n" "push %esp\n"
    "call _start_main\n" ".size _start, .-_start\n");
