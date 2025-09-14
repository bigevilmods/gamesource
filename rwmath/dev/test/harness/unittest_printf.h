// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_UNITTEST_UNITTEST_PRINTF_H
#define RW_MATH_UNITTEST_UNITTEST_PRINTF_H


void Printf(const char* format, ...);

//
// This code is useful for debugging math test failures.
//

template <class T>
static inline void PrintVectorIntrinsic(const T &v)
{
    union
    {
        struct
        {
            float x, y, z, w;
        } floats;
        T vi;
    } viUnion;

    viUnion.vi = v;
    Printf("[%f %f %f %f]\n", viUnion.floats.x, viUnion.floats.y, viUnion.floats.z, viUnion.floats.w);
}

template <class T>
static inline void PrintVectorIntrinsicHex(const T &v)
{
    union
    {
        struct
        {
            unsigned int x, y, z, w;
        } ints;
        T vi;
    } viUnion;

    viUnion.vi = v;
    Printf("[%x %x %x %x]\n", viUnion.ints.x, viUnion.ints.y, viUnion.ints.z, viUnion.ints.w);
}

template <class T>
static inline void PrintVector3(const T &v3)
{
    Printf("[%f %f %f]\n", (float)v3.GetX(), (float)v3.GetY(), (float)v3.GetZ());
}

template <class T>
static inline void PrintVector4(const T &v4)
{
    Printf("[%f %f %f %f]\n", (float)v4.GetX(), (float)v4.GetY(), (float)v4.GetZ(), (float)v4.GetW());
}


template <class T>
static inline void PrintMatrix33(const T &m)
{
    PrintVector3(m.xAxis);
    PrintVector3(m.yAxis);
    PrintVector3(m.zAxis);
}

template <class T>
static inline void PrintMatrix44Affine(const T &m)
{
    PrintVector3(m.xAxis);
    PrintVector3(m.yAxis);
    PrintVector3(m.zAxis);
    PrintVector3(m.wAxis);
}

template <class T>
static inline void PrintMatrix44(const T &m)
{
    PrintVector4(m.xAxis);
    PrintVector4(m.yAxis);
    PrintVector4(m.zAxis);
    PrintVector4(m.wAxis);
}

static inline const char* BoolAsString(bool value)
{
    return value ? "true" : "false";
}

static inline void PrintDescribedBool(const char *description, bool value)
{
    Printf("%s : %s\n", description, BoolAsString(value));
}

#endif // RW_MATH_UNITTEST_UNITTEST_PRINTF_H
