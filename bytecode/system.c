#ifndef SYSTEM_C
#define SYSTEM_C
#include <unistd.h>
#include <fcntl.h>
#include "system.h"
void my_exit(int status)
{
    _exit(status);
}
size_t my_read(int fd, void *buf, size_t count)
{
    char *buf_start = buf, *buf_done = buf;
    while (0 < count) {
	ssize_t rst = read(fd, buf_done, count);
	if (0 >= rst)
	    break;
	buf_done += rst;
	count -= rst;
    }
    return buf_done - buf_start;
}
void my_write(int fd, const void *buf, size_t count)
{
    write(fd, buf, count);
}
int my_open(const char *pathname)
{
    int flags = 0;
    return open(pathname, flags);
}
int my_close(int fd)
{
    return close(fd);
}
#endif				/* SYSTEM_C */
