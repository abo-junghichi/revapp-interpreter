#include <stdio.h>
int main(void)
{
    int i;
    for (i = 2; i <= 10; i++)
	printf("CONST_INT(%i, %i);\n", i, i);
    for (i = ' '; i <= '~'; i++)
	printf("CONST_INT(0x%x, const_0x%x);\n", i, i);
    return 0;
}
