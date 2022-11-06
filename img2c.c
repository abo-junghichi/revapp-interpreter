#include <stdio.h>
int main(void)
{
    const char *header = "static const char romsrc[] = {\n\t";
    while (1) {
	size_t i, rst;
	char buf[8];
	rst = fread(buf, 1, 8, stdin);
	for (i = 0; i < rst; i++) {
	    fprintf(stdout, "%s0x%02x", header, buf[i]);
	    header = ", ";
	}
	if (8 > rst)
	    break;
	header = ",\n\t";
    }
    fprintf(stdout, "%s')'\n};\n", header);
    return 0;
}
