#include <stdio.h>
#define LETTER "LETTERS_REVAPP"
int main(void)
{
    int i;
    char buf[2], *namep;
    printf("#ifndef " LETTER "\n" "#define " LETTER "\n\n"
	   "#include \"numbers.revapp\"\n\n" "(-- literal letters --)=\n\n"
	   "9='\\t'\n" "10='\\n'\n");
    buf[1] = '\0';
    namep = "\\s";
    for (i = ' '; i <= '~'; i++) {
	switch (i) {
	case ' ':
	    namep = "\\s";
	    break;
	case '\'':
	    namep = "\\'";
	    break;
	case '(':
	    namep = "brac";
	    break;
	case ')':
	    namep = "cket";
	    break;
	case '=':
	    namep = "eq";
	    break;
	case '\\':
	    namep = "\\\\";
	    break;
	default:
	    buf[0] = i;
	    namep = buf;
	}
	printf("const_0x%x='%s'\n", i, namep);
    }
    printf("\n#endif /* " LETTER "*/\n");
    return 0;
}
