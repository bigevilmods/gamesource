// (c) Electronic Arts. All Rights Reserved.
 
#ifndef RW_MATH_DETAIL_PS_UTIL_H
#define RW_MATH_DETAIL_PS_UTIL_H

namespace rw
{
namespace math
{

namespace ps
{

namespace detail
{

    typedef __vec2x32float__ mw_ps;

// This macro may be used to convert from a variable to a paired type.  Normally you can just cast
// a float to a paired type, but unfortunately the compiler does an unnecessary merge instruction.
// This macro exists to it is possible to either let the compiler do the conversion, or allow
// an inline assembly function to do the conversion.  Each has pros and cons.  So, this macro exists
// so that ether implementation could be used.  Note: to make this macro should only be used on
// floats that can have their address taken (i.e. it won't work on literals for an inline assembly
// implementation).
#define RW_MATH_PS_FROM_FLOAT_VAR(x) ((mw_ps)(x))

    // Sets a paired single to the two floating point values specified.
    //   ps0 provides the value for element 0
    //   ps1 provides the value for element 1
    extern inline mw_ps ps_set (const register float ps0, const register float ps1)
    {
        register mw_ps fd;
        asm
        {
            ps_merge00 fd, ps0, ps1;
        }
        return fd;
    }

    // Extracts element 0 of the vector as a float
    extern inline float ps_extract0(const register mw_ps ps01)
    {
        register float fd;
        asm
        {
            ps_merge00 fd, ps01, ps01;
        }
        return fd;
    }

    // Extracts element 1 of the vector as a float
    extern inline float ps_extract1(const register mw_ps ps01)
    {
        register float fd;
        asm
        {
            ps_merge11 fd, ps01, ps01;
        }
        return fd;
    }

#define RWMATH_WRAP_PS_LOAD_STORE(OFFSET) \
    extern inline mw_ps ps_load_ ## OFFSET(const register float *address) \
    { \
        register mw_ps result; \
        asm \
        { \
            psq_l result, OFFSET(address), 0, 0 \
        } \
        return result; \
    } \
    extern inline void ps_store_ ## OFFSET(const register mw_ps ps, const register float *address) \
    { \
        asm \
        { \
            psq_st ps, OFFSET(address), 0, 0 \
        } \
    } \
    extern inline void ps_store_as_float_ ## OFFSET(const register mw_ps ps, const register float *address) \
    { \
        asm \
        { \
            stfs ps, OFFSET(address) \
        } \
    }
    
    
    RWMATH_WRAP_PS_LOAD_STORE( 0);
    RWMATH_WRAP_PS_LOAD_STORE( 4);
    RWMATH_WRAP_PS_LOAD_STORE( 8);
    RWMATH_WRAP_PS_LOAD_STORE(12);
    RWMATH_WRAP_PS_LOAD_STORE(16);
    RWMATH_WRAP_PS_LOAD_STORE(20);
    RWMATH_WRAP_PS_LOAD_STORE(24);
    RWMATH_WRAP_PS_LOAD_STORE(28);
    RWMATH_WRAP_PS_LOAD_STORE(32);
    RWMATH_WRAP_PS_LOAD_STORE(36);
    RWMATH_WRAP_PS_LOAD_STORE(40);
    RWMATH_WRAP_PS_LOAD_STORE(44);
    RWMATH_WRAP_PS_LOAD_STORE(48);
    RWMATH_WRAP_PS_LOAD_STORE(52);
    RWMATH_WRAP_PS_LOAD_STORE(56);
    RWMATH_WRAP_PS_LOAD_STORE(60);

#undef RWMATH_WRAP_PS_LOAD_STORE

#define RWMATH_WRAP_INST_1( _INST ) \
    extern inline mw_ps _INST( const register mw_ps p0 ) \
    { \
        register mw_ps ret; \
        __asm { _INST ret, p0 }\
        return( ret ); \
    } \
    extern inline mw_ps _INST( const float p0 ) \
    { \
        register mw_ps ret; \
        register mw_ps psConst; \
        __asm \
        { \
            lfs psConst, p0; \
            _INST ret, psConst \
        } \
        return( ret ); \
    }  
 
#define RWMATH_WRAP_INST_2( _INST ) \
    extern inline mw_ps _INST( const register mw_ps p0, const register mw_ps p1 ) \
    { \
        register mw_ps ret; \
        __asm { _INST ret, p0, p1 } \
        return( ret ); \
    } \
    extern inline mw_ps _INST( const register mw_ps p0, const float p1 ) \
    { \
        register mw_ps ret; \
        register mw_ps psConst; \
        __asm \
        { \
             lfs psConst, p1; \
             _INST ret, p0, psConst \
        } \
        return( ret ); \
    } \
    extern inline mw_ps _INST( const float p0, const register mw_ps p1 ) \
    { \
        register mw_ps ret; \
        register mw_ps psConst; \
        __asm \
        { \
             lfs psConst, p0; \
             _INST ret, psConst, p1 \
        } \
        return( ret ); \
    }
    
#define RWMATH_WRAP_INST_3( _INST ) \
    extern inline mw_ps _INST( const register mw_ps p0, const register mw_ps p1, const register mw_ps p2 ) \
    { \
        register mw_ps ret; \
        __asm { _INST ret, p0, p1, p2 } \
        return( ret ); \
    } \
    extern inline mw_ps _INST( const register mw_ps p0, const register mw_ps p1, const float p2 ) \
    { \
        register mw_ps ret; \
        register mw_ps psConst; \
        __asm \
        { \
            lfs psConst, p2; \
            _INST ret, p0, p1, psConst \
        } \
        return( ret ); \
    } \
    extern inline mw_ps _INST( const register mw_ps p0, const float p1, const register mw_ps p2 ) \
    { \
        register mw_ps ret; \
        register mw_ps psConst; \
        __asm \
        { \
            lfs psConst, p1; \
            _INST ret, p0, psConst, p2 \
        } \
        return( ret ); \
    } \
    extern inline mw_ps _INST( const register mw_ps p0, const float p1, const float p2 ) \
    { \
        register mw_ps ret; \
        register mw_ps psConst1; \
        register mw_ps psConst2; \
        __asm \
        { \
            lfs psConst1, p1; \
            lfs psConst2, p2; \
            _INST ret, p0, psConst1, psConst2 \
        } \
        return( ret ); \
    } \
    extern inline mw_ps _INST( const float p0, const register mw_ps p1, const float p2 ) \
    { \
        register mw_ps ret; \
        register mw_ps psConst0; \
        register mw_ps psConst2; \
        __asm \
        { \
            lfs psConst0, p0; \
            lfs psConst2, p2; \
            _INST ret, psConst0, p1, psConst2 \
        } \
        return( ret ); \
    }

    RWMATH_WRAP_INST_1( ps_neg );
    RWMATH_WRAP_INST_1( ps_abs );   
    RWMATH_WRAP_INST_1( ps_nabs ); 
    RWMATH_WRAP_INST_1( ps_res );  
    RWMATH_WRAP_INST_1( ps_rsqrte );
     
    RWMATH_WRAP_INST_2( ps_add );
    RWMATH_WRAP_INST_2( ps_sub );
    RWMATH_WRAP_INST_2( ps_div );
    RWMATH_WRAP_INST_2( ps_merge00 );
    RWMATH_WRAP_INST_2( ps_merge01 );
    RWMATH_WRAP_INST_2( ps_merge10 );
    RWMATH_WRAP_INST_2( ps_merge11 );
    RWMATH_WRAP_INST_2( ps_mul );
    RWMATH_WRAP_INST_2( ps_muls0 );  
    RWMATH_WRAP_INST_2( ps_muls1 );  

    RWMATH_WRAP_INST_3( ps_madd );      
    RWMATH_WRAP_INST_3( ps_msub );   
    RWMATH_WRAP_INST_3( ps_nmadd );  
    RWMATH_WRAP_INST_3( ps_nmsub );  
    RWMATH_WRAP_INST_3( ps_sum0 );   
    RWMATH_WRAP_INST_3( ps_sum1 );   
    RWMATH_WRAP_INST_3( ps_madds0 ); 
    RWMATH_WRAP_INST_3( ps_madds1 ); 
    RWMATH_WRAP_INST_3( ps_sel );  

#undef RWMATH_WRAP_INST_1
#undef RWMATH_WRAP_INST_2
#undef RWMATH_WRAP_INST_3

#define RW_MATH_PS_DotV4(ret, in_vec1_ps0, in_vec1_ps1, in_vec2_ps0, in_vec2_ps1) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps mul_ps0_add_ps1 = ps_madd( in_vec1_ps0, in_vec2_ps0, ps_mul( in_vec1_ps1, in_vec2_ps1 )); \
        ret = ps_sum0( mul_ps0_add_ps1, mul_ps0_add_ps1, mul_ps0_add_ps1); \
        ret = ps_sum1( mul_ps0_add_ps1, ret, mul_ps0_add_ps1); \
    }


#define RW_MATH_PS_Lerp(ret_ps0, ret_ps1, q0_ps0, q0_ps1, q1_ps0, q1_ps1, vf_ps0, vf_ps1) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps dot_q0_q1; \
        RW_MATH_PS_DotV4(dot_q0_q1, q0_ps0, q0_ps1, q1_ps0, q1_ps1); \
        \
        mw_ps result_gt_zero_ps0 = ps_sub( q1_ps0, q0_ps0 ); \
        mw_ps result_gt_zero_ps1 = ps_sub( q1_ps1, q0_ps1 ); \
        result_gt_zero_ps0 = ps_madd( result_gt_zero_ps0, vf_ps0, q0_ps0 ); \
        result_gt_zero_ps1 = ps_madd( result_gt_zero_ps1, vf_ps1, q0_ps1 ); \
        \
        mw_ps result_lt_zero_ps0 = ps_add( q0_ps0, q1_ps0 ); \
        mw_ps result_lt_zero_ps1 = ps_add( q0_ps1, q1_ps1 ); \
        result_lt_zero_ps0 = ps_nmsub( result_lt_zero_ps0, vf_ps0, q0_ps0 ); \
        result_lt_zero_ps1 = ps_nmsub( result_lt_zero_ps1, vf_ps1, q0_ps1 ); \
        \
        ret_ps0 = ps_sel( dot_q0_q1, result_gt_zero_ps0, result_lt_zero_ps0 ); \
        ret_ps1 = ps_sel( dot_q0_q1, result_gt_zero_ps1, result_lt_zero_ps1 ); \
    }

#define RW_MATH_PS_ReciprocalEstimate(ret, in_vec) \
    { \
        using namespace ::rw::math::ps::detail; \
        ret = ps_res(in_vec); \
    }

#define RW_MATH_PS_ReciprocalFast(ret, in_vec) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps estimate = ps_res(in_vec); \
        ret = ps_madd(estimate, ps_nmsub(estimate, in_vec, 1.0f), estimate); \
    }

#define RW_MATH_PS_Reciprocal(ret, in_vec) \
    { \
        /* Do two iterations of Newton-Raphson estimation */ \
        using namespace rw::math::ps::detail; \
        mw_ps secondEstimate; \
        RW_MATH_PS_ReciprocalFast( secondEstimate, in_vec ); \
        ret = ps_madd(secondEstimate, ps_nmsub(secondEstimate, in_vec, 1.0f), secondEstimate); \
    }

#define RW_MATH_PS_ReciprocalSquareRootEstimate(ret, in_vec) \
    { \
        using namespace ::rw::math::ps::detail; \
        ret = ps_rsqrte(in_vec ); \
    }

#define RW_MATH_PS_ReciprocalSquareRootFast(ret, in_vec) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps estimate; \
        /* Do one iteration of Newton-Raphson estimation */ \
        estimate = ps_rsqrte(in_vec ); \
        ret = ps_madd( \
                          ps_mul( estimate, 0.5f ), \
                          ps_nmsub( in_vec, ps_mul(estimate,estimate), 1.0f), \
                          estimate \
                     ); \
    }

#define RW_MATH_PS_ReciprocalSquareRoot(ret, in_vec) \
    { \
        /* Do two iterations of Newton-Raphson estimation */ \
        using namespace rw::math::ps::detail; \
        mw_ps secondEstimate; \
        RW_MATH_PS_ReciprocalSquareRootFast( secondEstimate, in_vec ); \
        ret = ps_madd( \
                          ps_mul( secondEstimate, 0.5f ), \
                          ps_nmsub( in_vec, ps_mul(secondEstimate,secondEstimate), 1.0f), \
                          secondEstimate \
                     ); \
    }

#define RW_MATH_PS_SquareRoot(ret, in_vec) \
    { \
		using namespace rw::math::ps::detail; \
        mw_ps negIn = ps_neg( in_vec ); \
        mw_ps recipSqrt; \
        RW_MATH_PS_ReciprocalSquareRoot( recipSqrt, in_vec ); \
        mw_ps sqrt_result = ps_mul(in_vec, recipSqrt ); \
        /* If the input is > 0 then return the calculated sqrt.  If it is less */ \
        /* than or equal to 0, just return 0.                                  */ \
        ret = ps_sel( negIn, 0.0f, sqrt_result ); \
    }

#define RW_MATH_PS_SquareRootFast(ret, in_vec) \
    { \
		using namespace rw::math::ps::detail; \
        mw_ps negIn = ps_neg( in_vec ); \
        mw_ps recipSqrt; \
        RW_MATH_PS_ReciprocalSquareRootFast( recipSqrt, in_vec ); \
        mw_ps sqrt_result = ps_mul(in_vec, recipSqrt ); \
        /* If the input is > 0 then return the calculated sqrt.  If it is less */ \
        /* than or equal to 0, just return 0.                                  */ \
        ret = ps_sel( negIn, 0.0f, sqrt_result ); \
    }

#define RW_MATH_PS_SquareRootEstimate(ret, in_vec) \
    { \
		using namespace rw::math::ps::detail; \
        mw_ps negIn = ps_neg( in_vec ); \
        mw_ps recipSqrt; \
        RW_MATH_PS_ReciprocalSquareRootEstimate( recipSqrt, in_vec ); \
        mw_ps sqrt_result = ps_mul(in_vec, recipSqrt ); \
        /* If the input is > 0 then return the calculated sqrt.  If it is less */ \
        /* than or equal to 0, just return 0.                                  */ \
        ret = ps_sel( negIn, 0.0f, sqrt_result ); \
    }

#define RW_MATH_PS_NLerp(ret_ps0, ret_ps1, q0_ps0, q0_ps1, q1_ps0, q1_ps1, vf_ps0, vf_ps1) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps lerp_result_ps0, lerp_result_ps1; \
        RW_MATH_PS_Lerp(lerp_result_ps0, lerp_result_ps1, q0_ps0, q0_ps1, q1_ps0, q1_ps1, vf_ps0, vf_ps1); \
        \
        mw_ps dot_lerp_result; \
        RW_MATH_PS_DotV4(dot_lerp_result, lerp_result_ps0, lerp_result_ps1, lerp_result_ps0, lerp_result_ps1); \
        \
        mw_ps rsqrt_result; \
        RW_MATH_PS_ReciprocalSquareRoot(rsqrt_result, dot_lerp_result); \
        \
        ret_ps0 = ps_mul( lerp_result_ps0, rsqrt_result ); \
        ret_ps1 = ps_mul( lerp_result_ps1, rsqrt_result ); \
    }

#define RW_MATH_PS_NLerpFast(ret_ps0, ret_ps1, q0_ps0, q0_ps1, q1_ps0, q1_ps1, vf_ps0, vf_ps1) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps lerp_result_ps0, lerp_result_ps1; \
        RW_MATH_PS_Lerp(lerp_result_ps0, lerp_result_ps1, q0_ps0, q0_ps1, q1_ps0, q1_ps1, vf_ps0, vf_ps1); \
        \
        mw_ps dot_lerp_result; \
        RW_MATH_PS_DotV4(dot_lerp_result, lerp_result_ps0, lerp_result_ps1, lerp_result_ps0, lerp_result_ps1); \
        \
        mw_ps rsqrt_result; \
        RW_MATH_PS_ReciprocalSquareRootFast(rsqrt_result, dot_lerp_result); \
        \
        ret_ps0 = ps_mul( lerp_result_ps0, rsqrt_result ); \
        ret_ps1 = ps_mul( lerp_result_ps1, rsqrt_result ); \
    }

#define RW_MATH_PS_NLerpEstimate(ret_ps0, ret_ps1, q0_ps0, q0_ps1, q1_ps0, q1_ps1, vf_ps0, vf_ps1) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps lerp_result_ps0, lerp_result_ps1; \
        RW_MATH_PS_Lerp(lerp_result_ps0, lerp_result_ps1, q0_ps0, q0_ps1, q1_ps0, q1_ps1, vf_ps0, vf_ps1); \
        \
        mw_ps dot_lerp_result; \
        RW_MATH_PS_DotV4(dot_lerp_result, lerp_result_ps0, lerp_result_ps1, lerp_result_ps0, lerp_result_ps1); \
        \
        mw_ps rsqrt_result = ps_rsqrte( dot_lerp_result ); \
        \
        ret_ps0 = ps_mul( lerp_result_ps0, rsqrt_result ); \
        ret_ps1 = ps_mul( lerp_result_ps1, rsqrt_result ); \
    }

#define RW_MATH_PS_NormalizeEstimate(ret_ps0, ret_ps1, q_ps0, q_ps1) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps dot_result; \
        RW_MATH_PS_DotV4(dot_result, q_ps0, q_ps1, q_ps0, q_ps1); \
        \
        mw_ps rsqrt_result = ps_rsqrte( dot_result ); \
        \
        ret_ps0 = ps_mul( q_ps0, rsqrt_result ); \
        ret_ps1 = ps_mul( q_ps1, rsqrt_result ); \
    }

#define RW_MATH_PS_NormalizeFast(ret_ps0, ret_ps1, q_ps0, q_ps1) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps dot_result; \
        RW_MATH_PS_DotV4(dot_result, q_ps0, q_ps1, q_ps0, q_ps1); \
        \
        mw_ps rsqrt_result; \
        RW_MATH_PS_ReciprocalSquareRootFast(rsqrt_result, dot_result); \
        \
        ret_ps0 = ps_mul( q_ps0, rsqrt_result ); \
        ret_ps1 = ps_mul( q_ps1, rsqrt_result ); \
    }

#define RW_MATH_PS_Normalize(ret_ps0, ret_ps1, q_ps0, q_ps1) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps dot_result; \
        RW_MATH_PS_DotV4(dot_result, q_ps0, q_ps1, q_ps0, q_ps1); \
        \
        mw_ps rsqrt_result; \
        RW_MATH_PS_ReciprocalSquareRoot(rsqrt_result, dot_result); \
        \
        ret_ps0 = ps_mul( q_ps0, rsqrt_result ); \
        ret_ps1 = ps_mul( q_ps1, rsqrt_result ); \
    }

#define RW_MATH_PS_MultV4M44(ret_ps0, ret_ps1, v4_ps0, v4_ps1, xAxis_ps0, xAxis_ps1, yAxis_ps0, yAxis_ps1, zAxis_ps0, zAxis_ps1, wAxis_ps0, wAxis_ps1) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps in_x_ps0 = xAxis_ps0; \
        mw_ps v_ps0 = v4_ps0; \
        mw_ps r_ps0 = ps_muls0( in_x_ps0, v_ps0 ); /* 0 */ \
        mw_ps in_x_ps1 = xAxis_ps1; \
        mw_ps r_ps1 = ps_muls0( in_x_ps1, v_ps0 ); /* 1 */ \
        mw_ps in_y_ps0 = yAxis_ps0; \
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 ); /* 2 */ \
        mw_ps in_y_ps1 = yAxis_ps1; \
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 ); /* 3 */ \
        mw_ps v_ps1 = v4_ps1; \
        mw_ps in_z_ps0 = zAxis_ps0; \
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); /* 4 */ \
        mw_ps in_z_ps1 = zAxis_ps1; \
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 ); /* 5 */ \
        mw_ps in_w_ps0 = wAxis_ps0; \
        r_ps0 = ps_madds1( in_w_ps0, v_ps1, r_ps0 ); /* 6 */ \
        mw_ps in_w_ps1 = wAxis_ps1; \
        r_ps1 = ps_madds1( in_w_ps1, v_ps1, r_ps1 ); /* 7 */ \
        ret_ps0 = r_ps0; \
        ret_ps1 = r_ps1; \
    }

#define RW_MATH_PS_MultM44(r_x_ps0, r_x_ps1, r_y_ps0, r_y_ps1, r_z_ps0, r_z_ps1, r_w_ps0, r_w_ps1, m_x_ps0, m_x_ps1, m_y_ps0, m_y_ps1, m_z_ps0, m_z_ps1, m_w_ps0, m_w_ps1, b_x_ps0, b_x_ps1, b_y_ps0, b_y_ps1, b_z_ps0, b_z_ps1, b_w_ps0, b_w_ps1) \
    { \
        using namespace ::rw::math::ps::detail; \
        mw_ps in_x_ps0 = b_x_ps0; \
        mw_ps v_ps0 = m_x_ps0; \
        mw_ps r_ps0 = ps_muls0( in_x_ps0, v_ps0 ); /* 0 */ \
        mw_ps in_x_ps1 = b_x_ps1; \
        mw_ps r_ps1 = ps_muls0( in_x_ps1, v_ps0 ); /* 1 */ \
        mw_ps in_y_ps0 = b_y_ps0; \
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 ); /* 2 */ \
        mw_ps in_y_ps1 = b_y_ps1; \
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 ); /* 3 */ \
        mw_ps v_ps1 = m_x_ps1; \
        mw_ps in_z_ps0 = b_z_ps0; \
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); /* 4 */ \
        mw_ps in_z_ps1 = b_z_ps1; \
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 ); /* 5 */ \
        mw_ps in_w_ps0 = b_w_ps0; \
        r_ps0 = ps_madds1( in_w_ps0, v_ps1, r_ps0 ); /* 6 */ \
        mw_ps in_w_ps1 = b_w_ps1; \
        r_ps1 = ps_madds1( in_w_ps1, v_ps1, r_ps1 ); /* 7 */ \
        r_x_ps0 = r_ps0; \
        r_x_ps1 = r_ps1; \
        \
        v_ps0 = m_y_ps0; \
        r_ps0 = ps_muls0( in_x_ps0, v_ps0 ); /* 0 */ \
        r_ps1 = ps_muls0( in_x_ps1, v_ps0 ); /* 1 */ \
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 ); /* 2 */ \
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 ); /* 3 */ \
        v_ps1 = m_y_ps1; \
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); /* 4 */ \
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 ); /* 5 */ \
        r_ps0 = ps_madds1( in_w_ps0, v_ps1, r_ps0 ); /* 6 */ \
        r_ps1 = ps_madds1( in_w_ps1, v_ps1, r_ps1 ); /* 7 */ \
        r_y_ps0 = r_ps0; \
        r_y_ps1 = r_ps1; \
        \
        v_ps0 = m_z_ps0; \
        r_ps0 = ps_muls0( in_x_ps0, v_ps0 );  /* 0 */ \
        r_ps1 = ps_muls0( in_x_ps1, v_ps0 );  /* 1 */ \
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 );  /* 2 */ \
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 );  /* 3 */ \
        v_ps1 = m_z_ps1; \
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); /* 4 */ \
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 );  /* 5 */ \
        r_ps0 = ps_madds1( in_w_ps0, v_ps1, r_ps0 );  /* 6 */ \
        r_ps1 = ps_madds1( in_w_ps1, v_ps1, r_ps1 );  /* 7 */ \
        r_z_ps0 = r_ps0; \
        r_z_ps1 = r_ps1; \
        \
        v_ps0 = m_w_ps0; \
        r_ps0 = ps_muls0( in_x_ps0, v_ps0 );  /* 0 */ \
        r_ps1 = ps_muls0( in_x_ps1, v_ps0 );  /* 1 */ \
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 );  /* 2 */ \
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 );  /* 3 */ \
        v_ps1 = m_w_ps1; \
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 );  /* 4 */ \
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 );  /* 5 */ \
        r_ps0 = ps_madds1( in_w_ps0, v_ps1, r_ps0 );  /* 6 */ \
        r_ps1 = ps_madds1( in_w_ps1, v_ps1, r_ps1 );  /* 7 */ \
        r_w_ps0 = r_ps0; \
        r_w_ps1 = r_ps1; \
    }

#define RW_MATH_PS_InverseA4V3(ret_d, res_x_ps0, res_x_ps1, res_y_ps0, res_y_ps1, res_z_ps0, res_z_ps1, res_w_ps0, res_w_ps1, in_x_ps0, in_x_ps1, in_y_ps0, in_y_ps1, in_z_ps0, in_z_ps1, in_w_ps0, in_w_ps1 ) \
        /* Invert the upper left 3x3 submatrix. */                                                  \
        /*                                                                                          \
         * Note: The determinant of a Matrix44Affine is equal to the determinant of                 \
         * the upper left 3x3 submatrix.                                                            \
         *                                                                                          \
         * Proof:                                                                                   \
         *                                                                                          \
         * (Useful reference is found at http://mathworld.wolfram.com/Determinant.html)             \
         *                                                                                          \
         * The determinant of a matrix can be computed as:                                          \
         * det = sum from i=i to k of a_ij x C_ij        (Where C_ij is the cofactor of a_ij)       \
         *                                                                                          \
         * Lets choose j to be 4.                                                                   \
         * det = sum from i=i to k of a_i4 x C_i4        (Where C_ij is the cofactor of a_ij)       \
         *                                                                                          \
         * For an affine 4x4 matrix a_i1, a_i2, and a_i3 are 0, and a_i4 is 1.  So, the sum         \
         * can be rewritten as simply:                                                              \
         *                                                                                          \
         * det = C_4,4                                                                              \
         *     = -1^(4+4) x M_4,4                         (Where M_ij is the minor of a_ij)         \
         *     = M_4,4                                                                              \
         *                                                                                          \
         * Of course, this particular minor is just the determinant of the upper left 3x3 matrix.   \
         */                                                                                         \
        mw_ps m10_m00 = ps_merge00(in_y_ps0, in_x_ps0 ); \
        mw_ps m11_m01 = ps_merge11(in_y_ps0, in_x_ps0 ); \
        mw_ps m12_m02 = ps_merge00(in_y_ps1, in_x_ps1 ); \
        \
        mw_ps neg_in_y_ps0 = ps_neg(in_y_ps0); \
        mw_ps neg_in_z_ps0 = ps_neg(in_z_ps0); \
        mw_ps minort_x_ps0_partial = ps_muls1(m12_m02, neg_in_z_ps0); \
        mw_ps minort_x_ps0 = ps_madds0(m11_m01, in_z_ps1, minort_x_ps0_partial); \
        \
        mw_ps minort_y_ps0_partial = ps_muls0(m12_m02, neg_in_z_ps0); \
        mw_ps minort_y_ps0 = ps_madds0(m10_m00, in_z_ps1, minort_y_ps0_partial); \
        \
        mw_ps minort_z_ps0_partial = ps_muls0(m11_m01, neg_in_z_ps0); \
        mw_ps minort_z_ps0 = ps_madds1(m10_m00, in_z_ps0, minort_z_ps0_partial); \
        \
        mw_ps minort_yx_row_z_partial = ps_muls0(neg_in_y_ps0, in_x_ps1); \
        mw_ps minort_yx_row_z = ps_madds0(in_x_ps0, in_y_ps1, minort_yx_row_z_partial); \
        \
        mw_ps minort_z_row_z_partial = ps_muls1(neg_in_y_ps0, in_x_ps0); \
        mw_ps minort_z_row_z = ps_madds1(in_x_ps0, in_y_ps0, minort_z_row_z_partial); \
        \
        mw_ps det_temp0 = ps_muls1(minort_y_ps0, in_x_ps0); \
        mw_ps det_temp1 = ps_msub(minort_x_ps0, in_x_ps0, det_temp0); \
        mw_ps det_temp2 = ps_madd(minort_z_ps0, in_x_ps1, det_temp1); \
        det_ps = ps_merge00(det_temp2, det_temp2); \
        \
        mw_ps det_recip; \
        RW_MATH_PS_ReciprocalFast( det_recip, det_ps ); \
        mw_ps det_neg_recip = ps_neg(det_recip); \
        res_z_ps1 = ps_mul(det_recip, minort_z_row_z); \
        mw_ps det_recip_neg_pos = ps_merge00(det_neg_recip, det_recip); \
        mw_ps det_recip_pos_neg = ps_merge00(det_recip, det_neg_recip); \
        res_x_ps0 = ps_mul(minort_x_ps0, det_recip_pos_neg); \
        res_y_ps0 = ps_mul(minort_y_ps0, det_recip_neg_pos); \
        res_z_ps0 = ps_mul(minort_z_ps0, det_recip_pos_neg); \
        mw_ps minort_yx_row_z_scaled = ps_mul(det_recip_neg_pos, minort_yx_row_z); \
        res_y_ps1 = minort_yx_row_z_scaled; \
        res_x_ps1 = ps_merge11(minort_yx_row_z_scaled, minort_yx_row_z_scaled); \
        \
        \
        /* Now compute the translation value for the result that will cancel                        \
         * the translation component of the input.                                                  \
         */ \
        mw_ps negpos_ps0 = ps_neg(in_w_ps0); \
        mw_ps negpos_ps1 = ps_neg(in_w_ps1); \
        \
        res_w_ps0 = ps_muls0( res_x_ps0, negpos_ps0 ); /*X*/ \
        res_w_ps1 = ps_muls0( res_x_ps1, negpos_ps0 ); /*X*/ \
        res_w_ps0 = ps_madds1( res_y_ps0, negpos_ps0, res_w_ps0 ); /*Y*/ \
        res_w_ps1 = ps_madds1( minort_yx_row_z_scaled, negpos_ps0, res_w_ps1 ); /*Y*/ \
        res_w_ps0 = ps_madds0( res_z_ps0, negpos_ps1, res_w_ps0 ); /*Z*/ \
        res_w_ps1 = ps_madds0( res_z_ps1, negpos_ps1, res_w_ps1 ); /*Z*/

} // namespace detail
} // namespace ps
} // namespace math
} // namespace rw
#endif
