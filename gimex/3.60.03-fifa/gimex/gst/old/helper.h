// (c) 2003-2007 Electronic Arts Inc.

static inline int IntRound(float x)
{
#if defined(_MSC_VER)
#if NOSSE
	int r;
	__asm
    {
        fld     dword ptr x
        fistp   dword ptr r
    }
    return(r);
#else
    __asm
    {
        cvtss2si    eax,x
    }
	#endif
#else
	return ((int)(((float)(x))+((((float)(x))>=0.0f)?0.5f:-0.5f)));
#endif
}

static inline int IntMin(int a, int b)
{
    return(a < b ? a : b);
}

static inline int IntMax(int a, int b)
{
    return(a > b ? a : b);
}

static inline float FloatMin(float a, float b)
{
    return(a < b ? a : b);
}

static inline float FloatMax(float a, float b)
{
    return(a > b ? a : b);
}
