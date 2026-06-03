static int shra(int pas, int act)
{
// https://github.com/Rupt/c-arithmetic-right-shift
/*
    const int logical = (-1 >> 1) > 0;
    unsigned int fixu = -(logical & (pas < 0));
    int fix = *(int *) &fixu;
    return (pas >> act) | (fix ^ (fix >> act));
*/
#if defined(__GNUC__) || defined(__TINYC__)
// gcc treats shift-right on signed with sign extension.
// See [ info gcc -> C Implementation -> Integers implementation ].
// for __TINYC__
// https://en.wikipedia.org/wiki/Tiny_C_Compiler
    return pas >> act;
#else
// https://stackoverflow.com/questions/31879878/how-can-i-perform-arithmetic-right-shift-in-c-in-a-portable-way
    unsigned int upas = pas, sign_bit = ~((unsigned int) ~0 >> 1);
    return ((upas ^ sign_bit) >> act) - (sign_bit >> act);
#endif				// defined(__GNUC__) || defined(__TINYC__)
}
