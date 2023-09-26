#include "system.h"
static void core(int fd)
{
    char buf[9];
    buf[8] = '\n';
    while (1) {
	size_t s = my_read(fd, buf, 8);
	my_write(my_stdout, buf, 9);
	if (8 > s)
	    break;
    }
}
static int strlen(const char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++);
    return i;
}
void printerr(char *msg)
{
    my_write(my_stderr, msg, strlen(msg));
}
int main(int argc, char **argv)
{
    int fd;
    if (2 != argc) {
	printerr("usage: ");
	printerr(argv[0]);
	printerr(" [input_file]\n");
	return 21;
    }
    fd = my_open(argv[1]);
    core(fd);
    my_close(fd);
    return 42;
}
