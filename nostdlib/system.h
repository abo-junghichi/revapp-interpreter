#include <stddef.h>
size_t my_read(void *buf, size_t count);
void my_write(int fd, const void *buf, size_t count);
void my_exit(int status);
