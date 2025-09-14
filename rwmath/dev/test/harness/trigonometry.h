// (c) Electronic Arts. All Rights Reserved.

static const float FSIN_EPSILON         = 1e-5f;
static const float FCOS_EPSILON         = 1e-5f;
static const float FTAN_EPSILON         = 1e-3f;

//#if defined(EA_PLATFORM_XENON) && !defined(RWDEBUG)
//// 1e-6f value causes "internal compiler error" for some reason in release build
//const float FSINCOS_EPSILON      = 1e-5f;
//#else
static const float FSINCOS_EPSILON      = 1e-4f;
//#endif


// MathFpu epsilon vals for asin, acos, atan
static const float FASIN_EPSILON        = 1e-4f;
// values approaching 1.0 (0.9999) need slightly more
// error tolerance on the PC
static const float FACOS_EPSILON        = 1e-4f;
static const float FATAN_EPSILON        = 1e-4f;

//------------------------------------------------------------------------
// trig tables
//------------------------------------------------------------------------

struct UNARY_IN_UNARY_OUT_TRIG
{
    float input;
    float output;
};

struct UNARY_IN_BINARY_OUT
{
    float input;
    float output1;
    float output2;
};

struct BINARY_IN_UNARY_OUT
{
    float x;
    float y;
    float output;
};

static const UNARY_IN_BINARY_OUT sincos_values[] =
{
    {  0.000000f,  0.000000f,  1.000000f },
    {  0.123000f,  0.122690f,  0.992445f },
    {  3.141593f,  0.000000f, -1.000000f },
    {  3.041593f,  0.099833f, -0.995004f },
    {  3.241593f, -0.099833f, -0.995004f },
    {  6.283185f, -0.000000f,  1.000000f },
    {  6.183185f, -0.099833f,  0.995004f },
    {  6.383185f,  0.099833f,  0.995004f },
    {  1.570796f,  1.000000f,  0.000000f },
    {  1.470796f,  0.995004f,  0.099833f },
    {  1.670796f,  0.995004f, -0.099833f },
    {  0.785398f,  0.707107f,  0.707107f },
    {  0.685398f,  0.632981f,  0.774167f },
    {  0.885398f,  0.774167f,  0.632981f },
    { -3.141593f, -0.000000f, -1.000000f },
    { -3.241593f,  0.099833f, -0.995004f },
    { -3.041593f, -0.099833f, -0.995004f },
    { -6.283185f,  0.000000f,  1.000000f },
    { -6.383185f, -0.099833f,  0.995004f },
    { -6.183185f,  0.099833f,  0.995004f },
    { -1.570796f, -1.000000f,  0.000000f },
    { -1.670796f, -0.995004f, -0.099833f },
    { -1.470796f, -0.995004f,  0.099833f },
    { -0.785398f, -0.707107f,  0.707107f },
    { -0.885398f, -0.774167f,  0.632981f },
    { -0.685398f, -0.632981f,  0.774167f },
    {  100.0000f, -0.506365f,  0.862319f },
    { -100.0000f,  0.506366f,  0.862319f },
};

static const UNARY_IN_UNARY_OUT_TRIG tan_values[] =
{
    { 0.000000f, 0.000000f },
    { 0.010000f, 0.010000f },
    { 0.523599f, 0.577350f },
    { 0.785398f, 1.000000f },

    { 1.047198f, 1.732051f },
    { 2.094395f, -1.732051f },
    { 2.617994f, -0.577350f },
    { 1.560796f, 99.993399f },

    { 1.580796f, -99.999935f },
    { 2.356194f, -1.000000f },
    { 3.131593f, -0.010000f },
    { 3.151593f, 0.010000f },

    { -0.010000f, -0.010000f },
    { -0.523599f, -0.577350f },
    { -0.785398f, -1.000000f },
    { -1.047198f, -1.732051f },

    { -2.094395f, 1.732051f },
    { -2.617994f, 0.577350f },
    { -1.580796f, 99.999935f },
    { -1.560796f, -99.993399f },

    { -0.785398f, -1.000000f },
    { -3.151593f, -0.010000f },
    { -3.131593f, 0.0100000f },
    {  100.0000f, -0.587213f },
    { -100.0000f,  0.587214f },
};

static const UNARY_IN_BINARY_OUT asincos_values[] =
{
    { -0.999900f, -1.556654f,  3.127450f },
    { -0.707107f, -0.785398f,  2.356194f },
    { -0.500000f, -0.523599f,  2.094395f },
    { -0.000100f, -0.000100f,  1.570896f },
    {  0.000000f,  0.000000f,  1.570796f },
    {  0.000100f,  0.000100f,  1.570696f },
    {  0.500000f,  0.523599f,  1.047198f },
    {  0.707107f,  0.785398f,  0.785398f },
    {  0.999900f,  1.556654f,  0.014142f },
};

static const UNARY_IN_UNARY_OUT_TRIG atan_values[] =
{
    { -1000000000000000000.000000f, -1.570796f },
    { -1.000000f, -0.785398f },
    { 0.000000f, 0.000000f },
    { 0.500000f, 0.463648f },
    { 1.000000f, 0.785398f },
    { 1.570796f, 1.003885f },
    { 2.000000f, 1.107149f },
    { 3.000000f, 1.249046f },
    { 4.000000f, 1.325818f },
    { 1000000000000000000.000000f, 1.570796f },
};

static const BINARY_IN_UNARY_OUT atan2_values[] =
{
    /* Value  0: */ { -1.000000f, 1000000000000000000.000000f, -0.000000f },
    /* Value  1: */ { -1.000000f, 1.000000f, -0.785398f },
    /* Value  2: */ { -0.100000f, -0.200000f, -2.677945f },
    /* Value  3: */ { 3.141593f, 2.000000f, 1.003885f },
    /* Value  4: */ { 100000000000000000.000000f, 0.100000f, 1.570796f },
    /* Value  5: */ { 1.000000f, 0.f, 1.570796f },
    /* Value  6: */ { 5.000000f, -0.f, 1.570796f },
    /* Value  7: */ { -5.000000f, 0.f, -1.570796f },
    /* Value  8: */ { -5.000000f, -0.f, -1.570796f },
    /* Value  9: */ { -0.000000f, -1.f, -3.141593f },
    /* Value 10: */ { -0.000000f, 1.f, 0.0f },
};

static const int NUM_SINCOS_VALUES  = sizeof(sincos_values) / sizeof(sincos_values[0]);
static const int NUM_TAN_VALUES     = sizeof(tan_values) / sizeof(tan_values[0]);
static const int NUM_ASINCOS_VALUES = sizeof(asincos_values) / sizeof(asincos_values[0]);
static const int NUM_ATAN_VALUES    = sizeof(atan_values) / sizeof(atan_values[0]);
static const int NUM_ATAN2_VALUES   = sizeof(atan2_values) / sizeof(atan2_values[0]);


/* RWPUBLICEND */
