#include <stddef.h>
static int my_stdin = 0, my_stdout = 1, my_stderr = 2;
void my_exit(int status);
size_t my_read(int fd, void *buf, size_t count);
void my_write(int fd, const void *buf, size_t count);
int my_open(const char *pathname);
int my_close(int fd);
