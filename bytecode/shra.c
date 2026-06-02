static int shra(int pas, int act)
{
    // https://stackoverflow.com/questions/31879878/how-can-i-perform-arithmetic-right-shift-in-c-in-a-portable-way
    return pas < 0 ? ~(~pas >> act) : pas >> act;
/*
#ifdef __GNUC__
// gcc treats shift-right on signed with sign extension.
// See [ info gcc -> C Implementation -> Integers implementation ].
    return pas >> act;
#else
    unsigned int upas = pas, sign_bit = ~((unsigned int) ~0 >> 1);
    return ((upas ^ sign_bit) >> act) - (sign_bit >> act);
#endif				// __GNUC__
*/
}
