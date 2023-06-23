#include <unistd.h>
#include <sys/syscall.h>
#include "system.h"
#define Syscall_core "int $0x80"
static int syscall3(int syscall, int arg1, int arg2, int arg3)
{
    int ret;
    __asm__ volatile (Syscall_core:"=a"(ret):"a"(syscall), "b"(arg1),
		      "c"(arg2), "d"(arg3):"memory");
    return ret;
}
size_t my_read(void *buf, size_t count)
{
    char *buf_start = buf, *buf_done = buf;
    while (0 < count) {
	int rst = syscall3(SYS_read, 0, buf_done, count);
	if (0 >= rst)
	    break;
	buf_done += rst;
	count -= rst;
    }
    return buf_done - buf_start;
}
void my_write(int fd, const void *buf, size_t count)
{
    syscall3(SYS_write, fd, buf, count);
}
void my_exit(int status)
{
    int syscall = SYS_exit, ret;
    __asm__ volatile (Syscall_core:"=a"(ret):"a"(syscall),
		      "b"(status):"memory");
}
