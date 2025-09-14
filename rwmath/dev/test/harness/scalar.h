// (c) Electronic Arts. All Rights Reserved.

static const float FLOG_EPSILON          =   1e-6f;
static const float FSQRT_EPSILON         =   1e-6f;

#if defined EA_PLATFORM_PS3 || (defined(EA_PLATFORM_PLAYBOOK) && defined(__ARM__))
    static const float FEXP_EPSILON          =   6e-3f;
#else
    static const float FEXP_EPSILON          =   1e-3f;
#endif

//------------------------------------------------------------------------

struct UNARY_IN_UNARY_OUT_SCAL
{
    float input;
    float output;
};

static const UNARY_IN_UNARY_OUT_SCAL log_values[] =
{
    { 0.001000f, -6.907755f },
    { 0.010000f, -4.605170f },
    { 0.100000f, -2.302585f },
    { 0.950000f, -0.051293f },
    { 1.000000f, 0.000000f },
    { 1.100000f, 0.095310f },
    { 1.300000f, 0.262364f },
    { 1.900000f, 0.641854f },
    { 2.000000f, 0.693147f },
    { 2.718282f, 1.000000f },
    { 5.800000f, 1.757858f },
    { 10.000000f, 2.302585f },
    { 7.389056f, 2.000000f },
    { 20.085537f, 3.000000f },
    { 20.000000f, 2.995732f },
    { 99.900000f, 4.604170f },
    { 1001.000000f, 6.908755f },
    { 2234.890000f, 7.711947f },
};

static const int NUM_LOG_VALUES = sizeof(log_values) / sizeof(UNARY_IN_UNARY_OUT_SCAL);

static const UNARY_IN_UNARY_OUT_SCAL exp_values[] =
{
    { -100.000000f, 0.000000f },
    { -99.200000f, 0.000000f },
    { -2.900000f, 0.055023f },
    { -1.000000f, 0.367879f },
    { -0.100000f, 0.904837f },
    { 0.000000f, 1.000000f },
    { 0.000100f, 1.000100f },
    { 0.001000f, 1.001001f },
    { 0.100000f, 1.105171f },
    { 0.500000f, 1.648721f },
    { 0.900000f, 2.459603f },
    { 1.000000f, 2.718282f },
    { 1.100000f, 3.004166f },
    { 0.693147f, 2.000000f },
    { 2.302585f, 10.000000f },
    { 10.000000f, 22026.465795f },
};

static const int NUM_EXP_VALUES = sizeof(exp_values) / sizeof(UNARY_IN_UNARY_OUT_SCAL);

static const UNARY_IN_UNARY_OUT_SCAL sqrt_values[] =
{
    { 0.000000f, 0.000000f },
    { 0.000100f, 0.010000f },
    { 0.100000f, 0.316228f },
    { 0.500000f, 0.707107f },
    { 0.900000f, 0.948683f },
    { 1.000000f, 1.000000f },
    { 1.100000f, 1.048809f },
    { 2.000000f, 1.414214f },
    { 4.000000f, 2.000000f },
    { 10.000000f, 3.162278f },
};

static const int NUM_SQRT_VALUES = sizeof(sqrt_values) / sizeof(UNARY_IN_UNARY_OUT_SCAL);
