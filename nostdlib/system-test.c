#include "system.h"
void _start(void)
{
    char buf[9];
    buf[8] = '\n';
    while (1) {
	size_t s = my_read(buf, 8);
	my_write(1, buf, 9);
	if (8 > s)
	    break;
    }
    my_exit(42);
}
